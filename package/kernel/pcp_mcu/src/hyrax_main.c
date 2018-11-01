/*H*********************************************************************
* FILENAME: hyrax_main.c
*
* DESCRIPTION:
*	Hyrax MCU interface driver
*
* NOTES:
*   Copyright (c) IntelliDesign, 2016.  All rights reserved.
*
* CHANGES:
*
* VERS-NO CR-NO     DATE    WHO DETAIL
*   1               25Nov16 WTO Created
*
*H*/

/***********************************************************************
*   INCLUDE FILES
***********************************************************************/

/*----- system files -------------------------------------------------*/

#include <linux/module.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/of_platform.h>
#include <linux/watchdog.h>

/*----- projects files -----------------------------------------------*/

#define GLOBAL
#include "hyrax_driver.h"
#undef GLOBAL
#include "hyrax_ioctl.h"
#include "hyrax_i2c.h"

/***********************************************************************
*   PUBLIC DECLARATIONS
***********************************************************************/

/*----- context ------------------------------------------------------*/

#define DEFAULT_SPI_TRANSFER_LENGTH		32

#define HYRAX_CHARGER_MAJOR        		242
#define HYRAX_CHARGER_NAME         		DEVICE_NAME

#define SI4463_PART_NUMBER				0x6344

/*----- prototypes ---------------------------------------------------*/

#ifdef SUPPORT_FS
static int hyrax_charger_release( struct inode *inode, struct file *file);
static int hyrax_charger_open( struct inode *inode, struct file *file);
#endif
static irqreturn_t hyrax_irq_handler(int irq, void *dev_id);

/*----- variables ----------------------------------------------------*/

#ifdef SUPPORT_FS
struct file_operations hyrax_charger_fops = {
    .owner = THIS_MODULE,
    .llseek = NULL,
    .read = NULL,
    .write = NULL,
    .readdir = NULL,
    .poll = NULL,
    .unlocked_ioctl = NULL,
    .mmap = NULL,
    .open = hyrax_charger_open,
    .flush = NULL,
    .release = hyrax_charger_release,
    .fsync = NULL,
    .fasync = NULL,
    .lock = NULL,
};
#endif

static const struct i2c_device_id hyrax_charger_id[] = {
    { "pcp105mcu", 1, },
    { }
};
MODULE_DEVICE_TABLE(i2c, hyrax_charger_id);

#ifdef SUPPORT_FS
static struct class *psClass;
static struct device *psDev;
#endif

/***********************************************************************
*   PUBLIC FUNCTIONS
***********************************************************************/

#ifdef SUPPORT_FS
/*f*********************************************************************
* NAME: static int hyrax_charger_open( ... )
*
* DESCRIPTION:
*   Handles the open call to the device driver
*
* INPUTS:
*   None.
*
* OUTPUTS:
*   None.
*
* NOTES:
*
*F*/

static int hyrax_charger_open( struct inode *inode, struct file *file)
{
    struct hyrax_priv *priv = 
        container_of(inode->i_cdev, struct hyrax_priv, cdev);

    file->private_data = priv;

	return (0);
}

/*f*********************************************************************
* NAME: static int hyrax_charger_release( ... )
*
* DESCRIPTION:
*   Handles the close call to the device driver
*
* INPUTS:
*   None.
*
* OUTPUTS:
*   None.
*
* NOTES:
*
*F*/

static int hyrax_charger_release( struct inode *inode, struct file *file)
{
    file->private_data = NULL;
	return 0;
}
#endif	//SUPPORT_FS

static int __exit hyrax_charger_remove(struct i2c_client *i2c)
{
    struct hyrax_priv *priv = dev_get_drvdata(&i2c->dev);

	hyrax_charger_destroy_sysfs(i2c);
#ifdef SUPPORT_WATCHDOG
	hyrax_close_wdt(i2c);
#endif	
#ifdef SUPPORT_FS
	device_destroy(psClass, MKDEV(HYRAX_CHARGER_MAJOR, 0));
	class_destroy(psClass);
	unregister_chrdev( HYRAX_CHARGER_MAJOR, HYRAX_CHARGER_NAME );
#endif
    gpiochip_remove(&priv->gc);

    irq_domain_remove(priv->irq_domain);

	kfree(priv);

	return 0;
}

static int hyrax_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	return 0;
}

static void hyrax_irq_mask(struct irq_data *d)
{
    struct hyrax_priv *priv = irq_data_get_irq_chip_data(d);

    priv->irq_mask &= ~(1 << (d->irq - priv->irq_base));
}

static void hyrax_irq_unmask(struct irq_data *d)
{
    struct hyrax_priv *priv = irq_data_get_irq_chip_data(d);

    priv->irq_mask |= 1 << (d->irq - priv->irq_base);
}

static void hyrax_irq_bus_lock(struct irq_data *d)
{
    struct hyrax_priv *priv = irq_data_get_irq_chip_data(d);

    mutex_lock(&priv->irq_lock);
}

static void hyrax_irq_bus_sync_unlock(struct irq_data *d)
{
    struct hyrax_priv *priv = irq_data_get_irq_chip_data(d);

    mutex_unlock(&priv->irq_lock);
}

static int hyrax_irq_set_type(struct irq_data *d, unsigned int type)
{
    struct hyrax_priv *priv = irq_data_get_irq_chip_data(d);
    uint16_t level = d->irq - priv->irq_base;
    uint16_t mask = 1 << level;

    if (!(type & IRQ_TYPE_EDGE_BOTH)) {
        dev_err(&priv->i2c->dev, "irq %d: unsupported type %d\n",
            d->irq, type);
        return -EINVAL;
    }

    if (type & IRQ_TYPE_EDGE_FALLING)
        priv->irq_trig_fall |= mask;
    else
        priv->irq_trig_fall &= ~mask;

    if (type & IRQ_TYPE_EDGE_RISING)
        priv->irq_trig_raise |= mask;
    else
        priv->irq_trig_raise &= ~mask;

    return 0;
}

static struct irq_chip hyrax_irq_chip = {
    .name           = "hyrax_gpio",
    .irq_mask       = hyrax_irq_mask,
    .irq_unmask     = hyrax_irq_unmask,
    .irq_bus_lock   = hyrax_irq_bus_lock,
    .irq_bus_sync_unlock    = hyrax_irq_bus_sync_unlock,
    .irq_set_type   = hyrax_irq_set_type,
};

static inline void activate_irq(int irq)
{
    irq_clear_status_flags(irq, IRQ_NOREQUEST | IRQ_NOPROBE);
}


static int hyrax_irqdomain_map(struct irq_domain *d,
        unsigned int irq, irq_hw_number_t hwirq)
{
    struct hyrax_priv *priv = (struct hyrax_priv *) d->host_data;

	priv->irq_base = irq;

	irq_set_chip_data(irq, priv);
	irq_set_chip_and_handler(irq, &hyrax_irq_chip,
				 handle_edge_irq);
	irq_set_nested_thread(irq, 1);
	activate_irq(irq);

    return 0;
}

static const struct irq_domain_ops hyrax_irq_ops = {
    .map = hyrax_irqdomain_map,
    .xlate = irq_domain_xlate_onetwocell,
};

static int hyrax_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	struct hyrax_priv *priv = container_of(chip, struct hyrax_priv, gc);

    return irq_create_mapping(priv->irq_domain, offset);
}

/*F*********************************************************************
* NAME: static int __devinit hyrax_charger_probe(struct spi_device *spi)
*
* DESCRIPTION:
*	Initialise the interface to the Hyrax charger
*
* INPUTS:
*
* OUTPUTS:
*   None.
*
* NOTES:
*
*F*/

static int hyrax_charger_probe(struct i2c_client *i2c,
										 const struct i2c_device_id *id)
{
    struct hyrax_priv *priv;
	int ret = 0;
	int irq;

/*----- Allocate private memory struct -------------------------------*/
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
	{
		ret = -ENOMEM;
		goto error_out;
	}
    dev_set_drvdata(&i2c->dev, priv);

/*----- Attempt to detect hardware -----------------------------------*/
	if ( HardwareProbe( &i2c->dev ) == FALSE )
	{
		printk( "%s: Error detecting MCU\n", __FUNCTION__ );
		ret = -ENODEV;
		goto error_out;
	}

/*----- Initialise sysfs interface -----------------------------------*/
	hyrax_charger_init_sysfs( i2c );
	priv->i2c = i2c;

/*----- Initialise watchdog ------------------------------------------*/
#ifdef SUPPORT_WATCHDOG
	hyrax_init_wdt( i2c );
#endif

/*----- Initialise gpio interrupt ------------------------------------*/
	mutex_init(&priv->irq_lock);
    priv->irq_domain = irq_domain_add_simple( NULL, 1, 0, &hyrax_irq_ops, priv);
	if ( !priv->irq_domain )
	{
        printk( KERN_ERR "%s: Error creating irq domain\n", dev_info );
		goto error_out;
	}

/*----- Initialise irq -----------------------------------------------*/
	if ( (ret=request_threaded_irq(i2c->irq, hyrax_irq_handler,
		hyrax_read_status,
		IRQF_TRIGGER_RISING | IRQF_SHARED, "hyrax_irq", priv)))
	{
        printk( KERN_ERR "%s: Error %d initialising irq %d\n", dev_info, ret, i2c->irq );
	}

/*----- Initialise gpio ----------------------------------------------*/
	priv->gc.label = "hyrax_mcu_gpio";
    priv->gc.request = gpiochip_generic_request;
    priv->gc.free = gpiochip_generic_free;
	priv->gc.base = -1;
	priv->gc.ngpio = 1;
	priv->gc.owner = THIS_MODULE;

    priv->gc.get = hyrax_gpio_get;
    priv->gc.direction_input = hyrax_direction_input;
	priv->gc.to_irq = hyrax_gpio_to_irq;

    ret = gpiochip_add(&priv->gc);
    if (ret)
        goto error_gpio;

	hyrax_read_status(0, priv);

#ifdef SUPPORT_FS
/*----- Register /dev/v2v device ------------------------------------*/
	cdev_init( &priv->cdev, &hyrax_charger_fops );
	if ( cdev_add(&priv->cdev, MKDEV(HYRAX_CHARGER_MAJOR,0), 1) != 0 )
    {
        printk( KERN_ERR "%s: Error registering character device\n", dev_info );
        ret = -EIO;
		goto error_class;
    }

/*----- Instantiate class and device so dev entry is created --------*/
	if ( IS_ERR(psClass=class_create(THIS_MODULE, dev_info)) )
	{
		ret = PTR_ERR(psClass);
		goto error_class;
	}
	else if ( IS_ERR(psDev=device_create(psClass, NULL, MKDEV(HYRAX_CHARGER_MAJOR, 0), NULL, dev_info)) )
	{
		ret = PTR_ERR(psDev);
		goto error_device;
	}
#endif
/*----- Initialise interface and start receiving --------------------*/
	return ret;
#ifdef SUPPORT_FS
error_device:
	class_destroy(psClass);
error_class:
	unregister_chrdev( HYRAX_CHARGER_MAJOR, HYRAX_CHARGER_NAME );
#endif
error_gpio:
    irq_domain_remove(priv->irq_domain);
error_out:
    dev_err(&i2c->dev, "probe failed\n");
	kfree(priv);
    return ret;
}

static irqreturn_t hyrax_irq_handler(int irq, void *dev_id)
{
    return IRQ_WAKE_THREAD;
}

static const struct of_device_id hyrax_charger_dt_ids[] = {
    { .compatible = "intellidesign,pcp105mcu", },
	{ }
};

MODULE_DEVICE_TABLE(of, hyrax_charger_dt_ids);

static struct i2c_driver hyrax_charger_driver = {
    .driver = {
        .name   = "pcp105mcu",
		.of_match_table = hyrax_charger_dt_ids,
    },
    .probe      = hyrax_charger_probe,
    .remove     = hyrax_charger_remove,
    .id_table   = hyrax_charger_id,
};

static int hyrax_charger_init(void)
{
    return i2c_add_driver(&hyrax_charger_driver);
}

static void __exit hyrax_charger_exit(void)
{
    i2c_del_driver(&hyrax_charger_driver);
}

subsys_initcall(hyrax_charger_init);
module_exit(hyrax_charger_exit);

MODULE_AUTHOR("Intellidesign");
MODULE_DESCRIPTION("Hyrax charger driver");
MODULE_LICENSE("GPL v2");

