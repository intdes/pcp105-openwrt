/*H**************************************************************************
* FILENAME: pcp105p2_init.c
*
* DESCRIPTION:
*	PCP105 P2 board initialisation
*
* NOTES:
*   Copyright (c) IntelliDesign, 2016.  All rights reserved.
*
* CHANGES:
*
* VERS-NO CR-NO     DATE    WHO DETAIL
*
*H*/

/***********************************************************************
*   INCLUDE FILES
***********************************************************************/

/*----- system files -------------------------------------------------*/

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
#include <linux/can/platform/mcp251x.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/spi/spi.h>

/*----- context ------------------------------------------------------*/

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

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)			(sizeof(x)/sizeof(x[0]))
#endif

#define P2_CAN_GPIO				9

/*----- variables ----------------------------------------------------*/

static const unsigned int iAT24Config = AT24_DEVICE_MAGIC(2048 / 8, 0);

static struct mpu_platform_data mpu_data = 
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

static struct at24_platform_data at24_data = {
	.page_size = 8,
};

#define MCP23S08_BASE			504

static struct mcp23s08_platform_data mpc23s08_data = {
	{ { 1, 0 },		// chip[8]  { is_present, pullups }
	  { 1, 0 },
	  { 1, 0 },
	  { 1, 0 },
	  { 1, 0 },
	  { 1, 0 },
	  { 1, 0 },
	  { 1, 0 } },
	MCP23S08_BASE,	// base
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

static struct mcp251x_platform_data hyrax_mcp2515_pdata = {
    .oscillator_frequency   = 16*1000*1000,
};

static struct spi_board_info spi1_board_info[] = {
    [0] = {
        .modalias   = "mcp2515",
        .platform_data  = &hyrax_mcp2515_pdata,
        .irq        = -1,
        .max_speed_hz   = 1*1000*1000,
        .bus_num    = 169,
        .mode       = SPI_MODE_0,
        .chip_select    = 0,
    },
};

//TODO: Update to set correct gpio values from external LED expander
static struct gpio_led pcp105_p2_leds[] = {	
        {
                .name = "gps_led",
                .gpio = MCP23S08_BASE+2,
                .default_trigger = "pps",
                .active_low = 0,
        },
        {
                .name = "heartbeat_led",
                .gpio = MCP23S08_BASE+3,
                .default_trigger = "heartbeat",
                .active_low = 1,
        },
};

static struct gpio_led_platform_data hyrax_leds_data = {
        .num_leds = ARRAY_SIZE(pcp105_p2_leds),
        .leds = pcp105_p2_leds,
};

static struct platform_device pcp105_leds_dev = {
        .name = "leds-gpio",
        .id = -1,
        .dev.platform_data = &hyrax_leds_data,
};

static struct platform_device *pcp105_devs[] __initdata = {
        &pcp105_leds_dev,
};

int pcp105p2_init(void) 
{  

/*----- Register I2C devices -----------------------------------------*/	
	at24_data.byte_len = BIT(iAT24Config & AT24_BITMASK(AT24_SIZE_BYTELEN));
	at24_data.flags =  iAT24Config & AT24_BITMASK(AT24_SIZE_FLAGS);

	i2c0_board_info[0].irq = gpio_to_irq(MPU_9250_IRQ);

    if ( i2c_register_board_info(0, i2c0_board_info, ARRAY_SIZE(i2c0_board_info) ) != 0 )
	{
		printk( "Error registering board info\n" );
	}

/*----- Register LEDs ------------------------------------------------*/	
	platform_add_devices(pcp105_devs, 1 );

/*----- Register SPI devices -----------------------------------------*/	
	spi1_board_info[0].irq = gpio_to_irq(P2_CAN_GPIO);
    spi_register_board_info(spi1_board_info, ARRAY_SIZE(spi1_board_info));

	return 0;
}  

