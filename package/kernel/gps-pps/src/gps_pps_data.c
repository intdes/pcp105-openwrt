
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pps-gpio.h> // Added for PPS support
/* PPS GPIO data */  

#define DRIVER_AUTHOR "<howard.brownlee@cgi.com>"
#define DRIVER_DESC "GPIO PPS Data for NTP"

static struct pps_gpio_platform_data pps_gpio_info = {  
	.assert_falling_edge = false,  
	.capture_clear = false,  
	.gpio_pin = 15,  
	.gpio_label = "gpio15",  
};  


static struct platform_device pps_gpio_device = {  
	.name = "pps-gpio",  

	.id = -1,  
	.dev = {  
		.platform_data = &pps_gpio_info  
	},  
};  

static int __init gps_pps_init(void) {  
	int err;  

	pr_info("%s:\n",__func__);  
	err = platform_device_register(&pps_gpio_device);  
	if (err) {  
		pr_err("%s: Could not register PPS_GPIO device\n",__func__);  
	} else {  
		pr_info("%s: PPS_GPIO device registered OK\n",__func__);  
	}  

	return 0;
}  

static void __exit gps_pps_exit(void)
{
	platform_device_unregister(&pps_gpio_device);
}

module_init(gps_pps_init);
module_exit(gps_pps_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
