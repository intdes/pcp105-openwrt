#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/iio_mpu.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/platform_data/at24.h>
#include <linux/spi/mcp23s08.h>
#include <linux/platform_device.h>
#include <linux/leds.h>

/*----- context ------------------------------------------------------*/

#define DRIVER_AUTHOR "<will@intellidesign.com.au>"
#define DRIVER_DESC "PCP105 I2C Instantiation"

#define GYRO_ORIENTATION {  0,  1,  0,  1,  0,  0,  0,  0, -1 }
#define ACCEL_ORIENTATION { -1,  0,  0,  0,  1,  0,  0,  0, -1 }
#define COMPASS_ORIENTATION {  0,  0,  1,  0,  1,  0, -1,  0,  0 }
#define PRESSURE_ORIENTATION {  1,  0,  0,  0,  1,  0,  0,  0,  1 }

#define MPU_9250_IRQ			12

#define AT24_SIZE_BYTELEN 		5
#define AT24_SIZE_FLAGS 		8

#define AT24_BITMASK(x) (BIT(x) - 1)

/* create non-zero magic value for given eeprom parameters */
#define AT24_DEVICE_MAGIC(_len, _flags)         \
    ((1 << AT24_SIZE_FLAGS | (_flags))      \
	        << AT24_SIZE_BYTELEN | ilog2(_len))

#define PCP105					26819
#define FIT_MACHINE				28823

#define HWREV_P1                0x91
#define HWREV_P2                0x92

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)			(sizeof(x)/sizeof(x[0]))
#endif

/*----- prototypes ---------------------------------------------------*/

int GetHwRev( void );

/*----- variables ----------------------------------------------------*/

int iPCAId = 0;
int iPCARevision = 0;

module_param (iPCAId, uint, 0644);
module_param (iPCARevision, uint, 0644);

const unsigned int iAT24Config = AT24_DEVICE_MAGIC(2048 / 8, 0);

struct mpu_platform_data mpu_data = 
{
	.int_config  = 0x10,
    .level_shifter = 0,
    .orientation = ACCEL_ORIENTATION,
    .sec_slave_type = SECONDARY_SLAVE_TYPE_COMPASS,
    .sec_slave_id   = COMPASS_ID_AK8963,
    .secondary_i2c_addr = 0x18 >> 1,
    .secondary_orientation = COMPASS_ORIENTATION,
    .power_supply = NULL,
    .secondary_power_supply = NULL,
    .gpio = MPU_9250_IRQ,
};

struct at24_platform_data at24_data = {
	.page_size = 8,
};

struct mcp23s08_platform_data mpc23s08_data = {
	{ { 1, 0 },		// chip[8]  { is_present, pullups }
	  { 1, 0 },
	  { 1, 0 },
	  { 1, 0 },
	  { 1, 0 },
	  { 1, 0 },
	  { 1, 0 },
	  { 1, 0 } },
	-1,				// base
	false,			// irq_controller
	false,			// mirror
};

static struct i2c_board_info i2c0_board_info[] __initdata = {
{
		.type = "mpu9250",
		.addr = 0xD0 >> 1,
        .platform_data = &mpu_data,
},
{
		.type = "slb9645tt",
		.addr = 0x20,
},
{
		.type = "mcp23008",
		.addr = 0x21,
		.platform_data = &mpc23s08_data,
},
{
		.type = "bno055",
		.addr = 0x28,
},
{
		.type = "lm73",
		.addr = 0x48,
},
{
		.type = "ads1015",
		.addr = 0x4b,
},
{
		.type = "24c02",
		.addr = 0x50,
		.platform_data = &at24_data,
},

{
		.type = "pcp105mcu",
		.addr = 0x66,
},

};

static struct gpio_led pcp105_p1_leds[] = {
        {
                .name = "gps_led",
                .gpio = 1,
                .default_trigger = "pps",
                .active_low = 0,
        },
};

static struct gpio_led pcp105_p2_leds[] = {
        {
                .name = "gps_led",
                .gpio = 1,
                .default_trigger = "pps",
                .active_low = 0,
        },
        {
                .name = "heartbeat_led",
                .gpio = 2,
                .default_trigger = "heartbeat",
                .active_low = 0,
        },
};

static struct gpio_led fit_machine_p1_leds[] = {
        {
                .name = "gps_led",
                .gpio = 1,
                .default_trigger = "pps",
                .active_low = 0,
        },
        {
                .name = "heartbeat_red",
                .gpio = 13,
                .active_low = 0,
                .default_trigger = "none",
        },
        {
                .name = "heartbeat_blue",
                .gpio = 14,
                .active_low = 0,
                .default_trigger = "none",
        },
        {
                .name = "heartbeat_green",
                .gpio = 0,
                .active_low = 0,
                .default_trigger = "none",
        },
};


static struct gpio_led_platform_data hyrax_leds_data = {
        .num_leds = 1,
        .leds = pcp105_p1_leds,
};

static struct platform_device pcp105_leds_dev = {
        .name = "leds-gpio",
        .id = -1,
        .dev.platform_data = &hyrax_leds_data,
};

static struct platform_device *pcp105_devs[] __initdata = {
        &pcp105_leds_dev,
};

static char *IdString( int id )
{
	static char zString[32] = "";

	switch ( id )
	{
	case ( PCP105 ) :
		strcpy( zString, "PCP105" );
		break;
	case ( FIT_MACHINE ) :
		strcpy( zString, "Fit" );
		break;
	default :
		strcpy( zString, "Unknown" );
		break;
	}
	return ( zString );
}

static char *RevisionString( int iRevision )
{
	static char zString[32] = "";

	if ( ( iRevision >= 0x91 ) && ( iRevision < 0xa0 ) )
	{
		sprintf( zString, "P%d", iRevision-0x90 );
	}
	else if ( ( iRevision >= 0xa0 ) && ( iRevision <= (0xa0+('Z'-'A')) ) )
	{
		sprintf( zString, "%c", (iRevision-0xa0)+'A' );
	}
	else
	{
		strcpy( zString, "Unknown" );
	}
	return ( zString );
}

static int pcp_i2c_init(void) 
{  
	at24_data.byte_len = BIT(iAT24Config & AT24_BITMASK(AT24_SIZE_BYTELEN));
	at24_data.flags =  iAT24Config & AT24_BITMASK(AT24_SIZE_FLAGS);

	i2c0_board_info[0].irq = gpio_to_irq(MPU_9250_IRQ);

    if ( i2c_register_board_info(0, i2c0_board_info, ARRAY_SIZE(i2c0_board_info) ) != 0 )
	{
		printk( "Error registering board info\n" );
	}

	printk( "%s PCAId %d(%s), rev %Xh(%s)\n", __FUNCTION__, 
			iPCAId, IdString(iPCAId), 
			iPCARevision, RevisionString(iPCARevision) );
	switch ( iPCAId )
	{

/*----- Handle PCP105 ------------------------------------------------*/	
	default :
	case ( PCP105 ) :
		switch ( iPCARevision )
		{
		case ( HWREV_P1 ) :
			hyrax_leds_data.num_leds = ARRAY_SIZE(pcp105_p1_leds);
			hyrax_leds_data.leds = pcp105_p1_leds;
			break;
		default :
		case ( HWREV_P2 ) :
			hyrax_leds_data.num_leds = ARRAY_SIZE(pcp105_p2_leds);
			hyrax_leds_data.leds = pcp105_p2_leds;
			break;
		}
		break;

/*----- Handle Movus router ------------------------------------------*/	
	case ( FIT_MACHINE ) :
		switch ( iPCARevision )
		{
		default :
		case ( HWREV_P1 ) :
			hyrax_leds_data.num_leds = ARRAY_SIZE(fit_machine_p1_leds);
			hyrax_leds_data.leds = fit_machine_p1_leds;
			break;
		}
		break;
	}
	platform_add_devices(pcp105_devs, 1 );

	return 0;
}  

static void __exit pcp_i2c_exit(void)
{
}

module_init(pcp_i2c_init);
module_exit(pcp_i2c_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

