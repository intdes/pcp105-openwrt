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
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

/*----- projects files -----------------------------------------------*/

#include "hyrax_driver.h"

/***********************************************************************
*   PUBLIC DECLARATIONS
***********************************************************************/

static int iFileOffset = 0;

/*----- prototypes ---------------------------------------------------*/

struct file* file_open(const char* path, int flags, int rights) 
{
    struct file* filp = NULL;
    mm_segment_t oldfs;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
	iFileOffset = 0;
    if(IS_ERR(filp)) {
		printk( "Error oepning file ret with error %d\n", (long)filp);	//WTO
//        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file* file) 
{
    filp_close(file, NULL);
}

int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) 
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, (char *) data, size, (loff_t *) &offset);

    set_fs(oldfs);
    return ret;
}   

int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) 
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, (char *) data, size, (loff_t *) &offset);

    set_fs(oldfs);
    return ret;
}

int file_sync(struct file* file) 
{
    vfs_fsync(file, 0);
    return 0;
}

int freadline( struct file *file, char *pzLine, int iLineLength )
{
	int iPos = 0;
	int iRet = 0;
	char cByte;

	while ( iPos < iLineLength )
	{
		if ( (iRet=file_read(file, iFileOffset, (unsigned char *) &cByte, 1)) <= 0 )
		{
			break;
		}
		iFileOffset++;
		if ( cByte == '\n' )
		{
			break;
		}
		*pzLine++ = cByte;
		iPos++;
	}	
	return ( iRet );
}

