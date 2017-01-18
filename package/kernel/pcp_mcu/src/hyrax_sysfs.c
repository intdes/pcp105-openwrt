/*H*********************************************************************
* FILENAME: hyrax_sysfs.c
*
* DESCRIPTION:
*	Hyrax charger sysfs interface.
*
* NOTES:
*   Copyright (c) IntelliDesign, 2013.  All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/i2c.h>
#include <linux/watchdog.h>

/*----- projects files -----------------------------------------------*/

#include "hyrax_driver.h"
#include "hyrax_i2c.h"

/***********************************************************************
*   PUBLIC DECLARATIONS
***********************************************************************/

/*----- prototypes ---------------------------------------------------*/

static ssize_t hyrax_kick_watchdog(struct device *dev,
	                            struct device_attribute *attr,
    	                        const char *buf, size_t count);
static ssize_t hyrax_init_watchdog(struct device *dev,
	                            struct device_attribute *attr,
    	                        const char *buf, size_t count);
static ssize_t hyrax_start_upload(struct device *dev,
	                            struct device_attribute *attr,
    	                        const char *buf, size_t count);
static ssize_t hyrax_get_revision(struct device *dev,
                              struct device_attribute *attr, char *buf);
static ssize_t hyrax_read_watchdog_timeout(struct device *dev,
                              struct device_attribute *attr, char *buf);

static ssize_t hyrax_set_state(struct device *dev,
                    	       	  struct device_attribute *attr,
                        	   	  const char *buf, size_t count);
static ssize_t hyrax_get_state(struct device *dev,
                              struct device_attribute *attr, char *buf);

/*----- variables ----------------------------------------------------*/

static DEVICE_ATTR(init_watchdog, S_IWUSR|S_IWGRP|S_IRUSR|S_IRGRP, hyrax_read_watchdog_timeout, hyrax_init_watchdog);
static DEVICE_ATTR(kick_watchdog, S_IWUSR|S_IWGRP, NULL, hyrax_kick_watchdog);
static DEVICE_ATTR(upload, S_IWUSR|S_IWGRP, NULL, hyrax_start_upload);
static DEVICE_ATTR(revision, S_IRUSR|S_IRGRP, hyrax_get_revision, NULL);

static DEVICE_ATTR(state, 0644, hyrax_get_state, hyrax_set_state);

/***********************************************************************
*   PRIVATE DECLARATIONS
***********************************************************************/

/*----- context ------------------------------------------------------*/

/*----- prototypes ---------------------------------------------------*/

/***********************************************************************
*   PUBLIC FUNCTIONS
***********************************************************************/

/*F*********************************************************************
* NAME: int hyrax_charger_init_sysfs( struct i2c_client *i2c )
*
* DESCRIPTION:
*	Create sysfs interface.
*
* INPUTS:
*	None.
*
* OUTPUTS:
*	int					0 on success
*						<0 on error
*
* NOTES:
*
*F*/

int hyrax_charger_init_sysfs( struct i2c_client *i2c )
{
	int ret;

    ret = device_create_file(&i2c->dev, &dev_attr_upload);
    ret = device_create_file(&i2c->dev, &dev_attr_revision);

    ret = device_create_file(&i2c->dev, &dev_attr_state);
    
	ret = device_create_file(&i2c->dev, &dev_attr_init_watchdog);
    ret = device_create_file(&i2c->dev, &dev_attr_kick_watchdog);
	return ( ret );
}

void hyrax_charger_destroy_sysfs( struct i2c_client *i2c )
{
    device_remove_file(&i2c->dev, &dev_attr_upload);
    device_remove_file(&i2c->dev, &dev_attr_revision);
    
	device_remove_file(&i2c->dev, &dev_attr_state);
	
	device_remove_file(&i2c->dev, &dev_attr_init_watchdog);
	device_remove_file(&i2c->dev, &dev_attr_kick_watchdog);
}

static ssize_t hyrax_get_revision(struct device *dev,
                              struct device_attribute *attr, char *buf)
{
	int iRevision = GetRevision( dev );
	return sprintf( buf, "%d.%d\n", iRevision/100, iRevision%100 );
}

static ssize_t hyrax_get_state(struct device *dev,
                              struct device_attribute *attr, char *buf)
{
	return sprintf( buf, "%d\n", GetState( dev ) );
}

static ssize_t hyrax_set_state(struct device *dev,
                    	       	  struct device_attribute *attr,
                        	   	  const char *buf, size_t count)
{	
	int iState;

	sscanf( buf, "%d", &iState );

	SetState( dev, iState );

    return ( count );
}

/*F*********************************************************************
* NAME: ssize_t hyrax_kick_watchdog(...)
*
* DESCRIPTION:
*	Kicks the watchdog
*
* INPUTS:
*	None.
*
* OUTPUTS:
*
* NOTES:
*
*F*/

static ssize_t hyrax_kick_watchdog(struct device *dev,
	                            struct device_attribute *attr,
    	                        const char *buf, size_t count)
{
	KickWatchdog( dev );

	return ( count );
}

/*F*********************************************************************
* NAME: ssize_t hyrax_init_watchdog(...)
*
* DESCRIPTION:
*	Initialises the watchdog
*
* INPUTS:
*	None.
*
* OUTPUTS:
*
* NOTES:
*
*F*/


static ssize_t hyrax_init_watchdog(struct device *dev,
	                            struct device_attribute *attr,
    	                        const char *buf, size_t count)
{
	int iPeriod;

	sscanf( buf, "%d", &iPeriod );

	SetWatchdogTimeout( dev, iPeriod );

	return ( count );
}

static ssize_t hyrax_read_watchdog_timeout(struct device *dev,
                              struct device_attribute *attr, char *buf)
{
	return sprintf( buf, "%d\n", GetWatchdogTimeout( dev ) );
}

/*F*********************************************************************
* NAME: ssize_t v2v_start_upload(...)
*
* DESCRIPTION:
*	Requests a file to be uploaded to the charger bootloader
*
* INPUTS:
*	None.
*
* OUTPUTS:
*
* NOTES:
*
*F*/

static ssize_t hyrax_start_upload(struct device *dev,
                    	       	  struct device_attribute *attr,
                        	   	  const char *buf, size_t count)
{
	char zFileName[MAX_FILENAME_LENGTH+1];
	int i;

	strncpy( zFileName, buf, MAX_FILENAME_LENGTH );
	for ( i = 0; i < strlen(zFileName); i++ )
	{
		if ( ( zFileName[i] == '\r' ) || ( zFileName[i] == '\n' ) )
		{
			zFileName[i] = 0;
			break;
		}
	}
	LoadFirmware( dev, zFileName );

    return ( count );
}

