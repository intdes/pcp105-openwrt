/*H*********************************************************************
* FILENAME: hyrax_main.c
*
* DESCRIPTION:
*	Hyrax charger driver
*
* NOTES:
*   Copyright (c) IntelliDesign, 2014.  All rights reserved.
*
* CHANGES:
*
* VERS-NO CR-NO     DATE    WHO DETAIL
*   1               01Apr14 WTO Created
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
	hyrax_close_wdt(i2c);
#ifdef SUPPORT_FS
	device_destroy(psClass, MKDEV(HYRAX_CHARGER_MAJOR, 0));
	class_destroy(psClass);
	unregister_chrdev( HYRAX_CHARGER_MAJOR, HYRAX_CHARGER_NAME );
#endif
	kfree(priv);

	return 0;
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

/*----- Allocate private memory struct -------------------------------*/
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
	{
		ret = -ENOMEM;
		goto error_out;
	}

/*----- Initialise sysfs interface -----------------------------------*/
	hyrax_charger_init_sysfs( i2c );
    dev_set_drvdata(&i2c->dev, priv);
	priv->i2c = i2c;

/*----- Initialise watchdog ------------------------------------------*/
	hyrax_init_wdt( i2c );

#ifdef SUPPORT_FS
/*----- Register /dev/v2v device ------------------------------------*/
	cdev_init( &priv->cdev, &hyrax_charger_fops );
	if ( cdev_add(&priv->cdev, MKDEV(HYRAX_CHARGER_MAJOR,0), 1) != 0 )
    {
        printk( KERN_ERR "%s: Error registering character device\n", dev_info );
        ret = -EIO;
		goto error_probe;
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
    dev_err(&i2c->dev, "probe failed\n");
	kfree(priv);
error_out:
    return ret;
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

