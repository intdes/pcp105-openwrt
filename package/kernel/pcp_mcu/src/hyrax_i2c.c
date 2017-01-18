/*H*********************************************************************
* FILENAME: hyrax_i2c.c
*
* DESCRIPTION:
*	Hyrax charger I2C commands.
*
* NOTES:
*   Copyright (c) IntelliDesign, 2014.  All rights reserved.
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
#include <linux/slab.h>
#include <linux/watchdog.h>

/*----- projects files -----------------------------------------------*/

#include "hyrax_driver.h"
#include "hyrax_i2c.h"
#include "hyrax_file.h"

/***********************************************************************
*   PUBLIC DECLARATIONS
***********************************************************************/

/*----- context ------------------------------------------------------*/

#define APP_STATE					1
#define BOOTLDR_STATE				2

/***********************************************************************
*   PRIVATE DECLARATIONS
***********************************************************************/

/*----- context ------------------------------------------------------*/

#define EXPECTED_START          		0x2000
#define BOOTLOADER_START        		0x0000

#define ALLOWED_TRANSFER_ERRORS         3

#define DATA_RECORD                     0x00
#define MAX_LINE_DATA                   128
#define MAX_LINE_LENGTH					128
#define MAX_SEGMENTS                    128

#define MAX_TRANSFER_LENGTH				16

#define IVT_ADDR						0xFFF2
#define IVT_LEN							14

#define WRITE_DELAY						1

#define NUM_ATTEMPTS					5

#define PERFORM_UPLOAD

typedef struct
{
    WORD iOffset;
    WORD iStart;
    WORD iEnd;
    WORD iSize;
} seg_item_t;

/*----- prototypes ---------------------------------------------------*/

int FindSegments(const char *filename);
int FileExists( char *pzFileName );
int GetState( struct device *dev );
int SetState( struct device *dev, int iState );
int SetAddress( struct device *dev, WORD iAddress );
int GetAddress(struct device *dev);
int SetPacketLength( struct device *dev, WORD iLength );
void DumpMemory( int iAddress, BYTE *pData, int iBytes);
int ReadPacket( struct device *dev, BYTE *pData, WORD iPacketBytes );
int WritePacket( struct device *dev, BYTE *pData, WORD iPacketBytes );
int ResetFlash( struct device *dev );
BOOL SegExists( int iNumSegments, int iStart);
void UpdateLoadProgress( int iProgress );
void ReadSegment(BYTE *data, seg_item_t *psSegInfo, const char *filename);

/*----- variables ----------------------------------------------------*/

static seg_item_t asSegList[ MAX_SEGMENTS ];
int iGlobalTotalSize = 0;

/***********************************************************************
*   PUBLIC FUNCTIONS
***********************************************************************/

/*F*********************************************************************
* NAME: int GetRevision(struct device *dev)
*
* DESCRIPTION:
*	Reads the revision number from the charger.
*
* INPUTS:
*	None.
*
* OUTPUTS:
*	int				the revision number
*
* NOTES:
*
*F*/

int GetRevision( struct device *dev )
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iMajorRevision, iMinorRevision;

	iMajorRevision = i2c_smbus_read_word_data( i2c, IC_REG_BUILD_NUM_MAJOR );
	iMinorRevision = i2c_smbus_read_word_data( i2c, IC_REG_BUILD_NUM_MINOR );

	return ( ( iMajorRevision * 100 ) + iMinorRevision );
}

/*F*********************************************************************
* NAME: void KickWatchdog( struct device *dev )
*
* DESCRIPTION:
*	Kick the MCU watchdog.
*
* INPUTS:
*	dev					i2c device.
*
* OUTPUTS:
*
* NOTES:
*
*F*/

int KickWatchdog( struct device *dev )
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iRet;

	iRet = i2c_smbus_write_byte_data( i2c, IC_WATCHDOG_KICK, 0 );
	return ( iRet );
}

/*F*********************************************************************
* NAME: BOOL HardwareProbe( struct i2c_client *i2c )
*
* DESCRIPTION:
*	Probe for hardware
*
* INPUTS:
*	i2c			i2c device
*
* OUTPUTS:
*	BOOL		TRUE on success,
*				FALSE otherwise.
*
* NOTES:
*
*F*/

BOOL HardwareProbe( struct device *dev )
{
    struct hyrax_priv *priv = dev_get_drvdata(dev);
	BOOL bSuccess = FALSE;
	int iState;
	int i;

	for ( i = 0; i < NUM_ATTEMPTS; i++ )
	{
		iState = GetState( dev );
		if ( ( iState == APP_STATE ) || ( iState == BOOTLDR_STATE ) )
		{
			bSuccess = TRUE;
			break;
		}
		msleep( 100 );
	}
	priv->iRevision = 0;
	if ( bSuccess == TRUE )
	{
		priv->iRevision = GetRevision(dev);
	}
	return ( bSuccess );
}

/*F*********************************************************************
* NAME: void SetWatchdogTimeout( struct device *dev, WORD iTimeout )
*
* DESCRIPTION:
*	Sets the watchdog timeout on the MCU.
*
* INPUTS:
*	dev					I2C device
*	iTimeout			watchdog timeout.
*
* OUTPUTS:
*
* NOTES:
*
*F*/

int SetWatchdogTimeout( struct device *dev, WORD iTimeout )
{
    struct i2c_client *i2c = to_i2c_client(dev);
    struct hyrax_priv *priv = dev_get_drvdata(dev);
	int iRet;

/*----- The timeout was changes to seconds in Rev 4 ------------------*/
	if ( priv->iRevision < 400 )
	{
		iTimeout *= 1000;
	}
	iRet = i2c_smbus_write_word_data( i2c, IC_WATCHDOG_INIT, iTimeout );
	return ( iRet );
}

int GetWatchdogTimeout( struct device *dev )
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iTimeout;

	iTimeout = i2c_smbus_read_word_data( i2c, IC_WATCHDOG_INIT );

	return ( iTimeout );
}
/*F*********************************************************************
* NAME: int LoadFirmware( const char *pzFileName )
*
* DESCRIPTION:
*	Loads firmware contained in the given HEX file to the MSP430
*	charger.
*
* INPUTS:
*	pzFileName			file to load.
*
* OUTPUTS:
*   BOOL                TRUE if successful,
*                       FALSE otherwise.
*
* NOTES:
*
*F*/

int LoadFirmware( struct device *dev, char *pzFileName )
{
    BYTE aRxBuffer[256];
    int iNumSegments, iSegment;
    int iPacketBytes = 0;
    int iBytesLeft;
    int iBytesSent;
    BYTE *pData;
    BOOL bSuccess = FALSE;
    BOOL bFinished = FALSE;
    int iDownloadBytesSent = 0;
    int iErrors = 0;
	int iNewBuildNumber;
	int iOldBuildNumber = -1;
	char zStatus[256];
    int iInitState;
	int iRet;
    BOOL bReadError = FALSE;
    int iLastPacketBytes = -1;
	int iTestAddress;

/*----- Find initial state of device ---------------------------------*/
	if ( FileExists( pzFileName ) != 0 )
	{
		printk( KERN_ERR "Error opening file %s\n", pzFileName );
		bFinished = TRUE;
	}
	else
	{
		iNumSegments = FindSegments( pzFileName );
		if ( iNumSegments == 0 )
		{
			bFinished = TRUE;
		}

		iInitState = GetState(dev);
		iDownloadBytesSent = 0;
		if ( iInitState == APP_STATE )
		{
			iOldBuildNumber = GetRevision(dev);
		}
		else if ( iInitState == -1 )
		{
			printk( KERN_ERR "MSP430 in invalid state\n" );
			bFinished = TRUE;
		}

/*----- Verify input HEX file ----------------------------------------*/
		if ( ( asSegList[0].iStart < EXPECTED_START ) ||
			 ( SegExists( iNumSegments, BOOTLOADER_START ) == TRUE ) )
		{
			printk( KERN_ERR "Invalid HEX file\n" );
			bFinished = TRUE;
		}
	}
    while ( bFinished == FALSE )
    {
        msleep(500);

/*----- Start bootloader ---------------------------------------------*/
        if ( SetState( dev, BOOTLDR_STATE ) != 0 )
        {
            printk( KERN_ERR "Error setting bootloader state\n" );
            bFinished = TRUE;
            continue;
        }

/*----- Reset flash clear block state --------------------------------*/
        if ( ResetFlash(dev) != 0 )
        {
            printk( KERN_ERR "Error resetting flash state\n" );
            bFinished = TRUE;
            continue;
        }
        printk( KERN_INFO "Uploading and verifying..." );
        UpdateLoadProgress(0);

/*----- Perform upload -----------------------------------------------*/
#ifdef PERFORM_UPLOAD
        for ( iSegment = 0; iSegment < iNumSegments; iSegment++ )
        {
            if ( SetAddress( dev, asSegList[iSegment].iStart ) != 0 )
            {
                bFinished = TRUE;
                break;
            }


            pData = (BYTE *) kmalloc( asSegList[iSegment].iSize, GFP_KERNEL );
            ReadSegment( pData, &asSegList[iSegment], pzFileName );

            iBytesLeft = asSegList[iSegment].iSize;
            iBytesSent = 0;
            while ( iBytesLeft > 0 )
            {
                if ( iBytesLeft > MAX_TRANSFER_LENGTH )
                {
                    iPacketBytes = MAX_TRANSFER_LENGTH;
                }
                else
                {
                    iPacketBytes = iBytesLeft;
                }
                if ( iLastPacketBytes != iPacketBytes )
                {
                    if ( SetPacketLength( dev, iPacketBytes ) != 0 )
                    {
                        bFinished = TRUE;
                        break;
                    }
                    iLastPacketBytes = iPacketBytes;
                }
				if ( (iRet=WritePacket( dev, &pData[iBytesSent], iPacketBytes )) < 0 )
				{
					printk( KERN_ERR "WritePacket error %d\n", iRet );
				}
				iBytesSent += iPacketBytes;
                iDownloadBytesSent += iPacketBytes;
                iBytesLeft -= iPacketBytes;
                UpdateLoadProgress( iDownloadBytesSent*100/iGlobalTotalSize );
            }
            kfree( pData );
            if ( bFinished == TRUE )
            {
                break;
            }
        }
        if ( bFinished == TRUE )
        {
            continue;
        }
#endif

/*----- Perform verify -----------------------------------------------*/
        iErrors = 0;
        iLastPacketBytes = -1;
        for ( iSegment = 0; iSegment < iNumSegments; iSegment++ )
        {
            if ( SetAddress( dev, asSegList[iSegment].iStart ) != 0 )
            {
                printk( KERN_ERR "Error setting address\n" );
                bFinished = TRUE;
                break;
            }
			iTestAddress = GetAddress( dev );

            pData = (BYTE *) kmalloc( asSegList[iSegment].iSize, GFP_KERNEL );
            ReadSegment( pData, &asSegList[iSegment], pzFileName );

            iBytesLeft = asSegList[iSegment].iSize;
            iBytesSent = 0;
            while ( iBytesLeft > 0 )
            {
                if ( iBytesLeft > MAX_TRANSFER_LENGTH )
                {
                    iPacketBytes = MAX_TRANSFER_LENGTH;
                }
                else
                {
                    iPacketBytes = iBytesLeft;
                }
                if ( iLastPacketBytes != iPacketBytes )
                {
                    if ( SetPacketLength( dev, iPacketBytes ) != 0 )
                    {
						printk( "Error setting packet length\n" );
                        bFinished = TRUE;
                        break;
                    }
                    iLastPacketBytes = iPacketBytes;
                }

                bReadError = FALSE;
				if ( (iRet=ReadPacket( dev, aRxBuffer, iPacketBytes )) < 0 )
				{
                    printk( KERN_ERR "Error getting read response @ %Xh\n",
                         asSegList[iSegment].iStart + iBytesSent );
                    bReadError = TRUE;
				}
                if ( ( bReadError == FALSE ) &&
                     ( memcmp( aRxBuffer, &pData[iBytesSent], iPacketBytes ) != 0 ) )
                {
                    printk( KERN_ERR "Verify Error at address %04Xh\n",
                             asSegList[iSegment].iStart + iBytesSent );
                    DumpMemory( asSegList[iSegment].iStart + iBytesSent,
                                &aRxBuffer[0], iPacketBytes );
                    DumpMemory( asSegList[iSegment].iStart + iBytesSent,
                                &pData[iBytesSent], iPacketBytes );
                    bReadError = TRUE;
                }
                if ( bReadError == TRUE )
                {
                    if ( iErrors++ < ALLOWED_TRANSFER_ERRORS )
                    {
                        printk( KERN_ERR "Restarting at address %04Xh\n",
                             asSegList[iSegment].iStart + iBytesSent );
                        SetAddress( dev, asSegList[iSegment].iStart + iBytesSent );
                        msleep(100);
                        continue;
                    }
                    else
                    {
                        printk( KERN_ERR "Too many errors received\n" );
                        bFinished = TRUE;
                        break;
                    }
                }
                iErrors = 0;
                iBytesLeft -= iPacketBytes;
                iBytesSent += iPacketBytes;
                iDownloadBytesSent += iPacketBytes;
                UpdateLoadProgress( iDownloadBytesSent*100/iGlobalTotalSize );
            }
            kfree( pData );
            if ( bFinished == TRUE )
            {
                break;
            }
        }
        if ( bFinished == TRUE )
        {
            continue;
        }
/*----- Return to application ----------------------------------------*/
        if ( SetState( dev, APP_STATE ) != 0 )
        {
            printk( KERN_ERR "Error returning to application\n" );
        }
        else
        {
            bSuccess = TRUE;

/*----- Show upgrade information -------------------------------------*/
            msleep(250);
            iNewBuildNumber = GetRevision(dev);
            strcpy( zStatus, "Successfully upgraded " );
			if ( iOldBuildNumber > 0  )
            {
                sprintf( &zStatus[strlen(zStatus)], "from Rev %d.%d", iOldBuildNumber/100, 
																	  iOldBuildNumber%100);
            }
			if ( iNewBuildNumber > 0 )
            {
                sprintf( &zStatus[strlen(zStatus)], " to Rev %d.%d", iNewBuildNumber/100,
																	iNewBuildNumber%100);
            }
            printk( KERN_INFO "%s\n", zStatus );
        }
        bFinished = TRUE;
    }
    UpdateLoadProgress( -1 );
    if ( bSuccess == FALSE )
    {
        printk( KERN_ERR "Upload/verify failed\n" );
    }
    return ( bSuccess );
}

/***********************************************************************
*   PRIVATE FUNCTIONS
***********************************************************************/

/*F*********************************************************************
* NAME: int WritePacket( struct device *dev, BYTE *pData, 
*						 WORD iPacketBytes )
*
* DESCRIPTION:
*	Write the given packet to the charger
*
* INPUTS:
*	dev					linux device
*	pData				data pointer
*	iPacketBytes		number of bytes to write.
*
* OUTPUTS:
*	int					0 if successful, <0 otherwise.
*
* NOTES:
*
*F*/

int WritePacket( struct device *dev, BYTE *pData, WORD iPacketBytes )
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iRet;
	struct i2c_msg msg[1];
	BYTE aPacketData[MAX_TRANSFER_LENGTH+1];

	aPacketData[0] = IC_REG_BOOT_WRITE_DATA;
	memcpy( &aPacketData[1], pData, iPacketBytes );

	msg[0].addr = i2c->addr;
	msg[0].flags = 0;	
	msg[0].len = iPacketBytes+1;
	msg[0].buf = aPacketData;

	iRet = i2c_transfer(i2c->adapter, msg, 1 );
	if ( iRet == 1 )
	{
		iRet = iPacketBytes;
	}
	else
	{
		iRet = -1;
	}
	msleep(WRITE_DELAY);
	return ( iRet );
}

/*F*********************************************************************
* NAME: int ReadPacket( struct device *dev, BYTE *pData, 
*						 WORD iPacketBytes )
*
* DESCRIPTION:
*	Reads a packet from the charger
*
* INPUTS:
*	dev					linux device
*	pData				data pointer
*	iPacketBytes		number of bytes to read
*
* OUTPUTS:
*	int					0 if successful, <0 otherwise.
*
* NOTES:
*
*F*/

int ReadPacket( struct device *dev, BYTE *pData, WORD iPacketBytes )
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iRet;
	struct i2c_msg msg[2];
	BYTE iCommand = IC_REG_BOOT_READ_DATA;

	msg[0].addr = i2c->addr;
	msg[0].flags = 0;	
	msg[0].len = 1;
	msg[0].buf = &iCommand;
	msg[1].addr = i2c->addr;
	msg[1].flags = I2C_M_RD;	
	msg[1].len = iPacketBytes;
	msg[1].buf = pData;

	iRet = i2c_transfer(i2c->adapter, msg, 2 );
	if ( iRet == 2 )
	{
		iRet = iPacketBytes;
	}
	if ( iRet != iPacketBytes )
	{
		printk( "%s : Read %d bytes, expected %d bytes\n", __FUNCTION__,
				iRet, iPacketBytes );
		iRet = -1;
	}
	else
	{
		iRet = 0;
	}
	return ( iRet );
}

/*F*********************************************************************
* NAME: BOOL SetAddress( WORD iAddress )
*
* DESCRIPTION:
*   Sets the active transfer address to the given address.
*
* INPUTS:
*   iAddress            the address to set
*
* OUTPUTS:
*   BOOL                TRUE if successful,
*                       FALSE otherwise.
*
* NOTES:
*
*F*/

int SetAddress(struct device *dev, WORD iAddress)
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iRet;

	iRet = i2c_smbus_write_word_data( i2c, IC_REG_BOOT_BASE_ADDRESS, iAddress );
	return ( iRet );
}

int GetAddress(struct device *dev)
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iRet;

	iRet = i2c_smbus_read_word_data( i2c, IC_REG_BOOT_GET_BASE_ADDRESS );
	return ( iRet );
}

/*f*********************************************************************
* NAME: int ResetFlash( void )
*
* DESCRIPTION:
*   Reset flash erase map.
*
* INPUTS:
*   None.
*
* OUTPUTS:
*   BOOL            TRUE on success, FALSE otherwise.
*
* NOTES:
*   None.
*
*f*/

int ResetFlash( struct device *dev )
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iRet;

	iRet = i2c_smbus_write_byte_data( i2c, IC_REG_BOOT_RESET_MAP, 1);
	return ( iRet );
}

/*f*********************************************************************
* NAME: void SetPacketLength( WORD iLength )
*
* DESCRIPTION:
*   Set the length of the next transmitted/received packet
*
* INPUTS:
*   iLength         length of the next packet in bytes
*
* OUTPUTS:
*   None.
*
* NOTES:
*   None.
*
*f*/

int SetPacketLength( struct device *dev, WORD iLength )
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iRet;

	iRet = i2c_smbus_write_byte_data( i2c, IC_REG_BOOT_PACKET_LENGTH, 
									  iLength );
	return ( iRet );
}

/*F*********************************************************************
* NAME: int GetState(struct device *dev)
*
* DESCRIPTION:
*	Reads the current state of the device.
*
* INPUTS:
*	dev				linux device.
*
* OUTPUTS:
*	int				APP_STATE, BOOTLDR_STATE or <0 on error.
*
* NOTES:
*
*F*/

int GetState(struct device *dev)
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iState;

	iState = i2c_smbus_read_byte_data( i2c, IC_REG_DEVICE_STATE );

	return ( iState );
}

/*F*********************************************************************
* NAME: int SetState( struct device *dev, int iState )
*
* DESCRIPTION:
*	Sets the state of the charger
*
* INPUTS:
*	dev					linux device
*   iState              BOOTLDR_STATE or APP_STATE
*
* OUTPUTS:
*	int					0 on success, <0 on error
*
* NOTES:
*
*F*/

int SetState( struct device *dev, int iState )
{
    struct i2c_client *i2c = to_i2c_client(dev);
	int iRet;

	iRet = i2c_smbus_write_byte_data( i2c, IC_REG_DEVICE_STATE, iState );
	if ( ( iRet == 0 ) && ( iState == BOOTLDR_STATE ) )
	{
		msleep(500);
	}

	return ( iRet );
}

/*f*********************************************************************
* NAME: int FindSegments( char *filename )
*
* DESCRIPTION:
*   Parses an Intel HEX file to discover the number of segments, the
*   size of those segments and their start and end addresses.
*
* INPUTS:
*   None.
*
* OUTPUTS:
*   int             the number of segments
*
* NOTES:
*   None.
*
*f*/

int FindSegments(const char *filename)
{
    UINT iRecType;
	struct file *fsp;
    int bJoinedSegment = FALSE;
    int size, iStartAddr, iEndAddr;
	BOOL bFinished = FALSE;
	char zLine[MAX_LINE_LENGTH+1];
    int i;
    int iNumSegs;
    int iOldSize;
    int iTotalSize;
	int iRet;
	char zString[16];
	int iPos;

    fsp = file_open( filename, O_RDONLY, 0 );
    iNumSegs = 0;
	while ( bFinished == FALSE )
    {
		iRet = freadline( fsp, zLine, MAX_LINE_LENGTH );
		if ( iRet <= 0 )
		{
			bFinished = TRUE;
			continue;
		}
		iPos = 1;
		strncpy( zString, &zLine[iPos], 2 );
        sscanf( zString, "%02X", &size );
		iPos += 2;
		strncpy( zString, &zLine[iPos], 4 );
        sscanf( zString, "%04X", &iStartAddr );
		iPos += 4;
		strncpy( zString, &zLine[iPos], 2 );
        sscanf( zString, "%02X", &iRecType );

        if ( iRecType != DATA_RECORD )
        {
            continue;
        }
		if ( iStartAddr < EXPECTED_START )
		{
			printk( KERN_DEBUG "Skipping data at %Xh\n", iStartAddr );
			continue;
		}
        iEndAddr = iStartAddr+size-1;

/*----- Discover if segment is adjacent to the last segment ----------*/
        if ( iNumSegs == 0 )
        {
            bJoinedSegment = FALSE;
        }
        else if ( (UINT)( asSegList[iNumSegs-1].iEnd + 1 ) == iStartAddr )
        {
            bJoinedSegment = TRUE;
            iOldSize = asSegList[iNumSegs-1].iEnd - asSegList[iNumSegs-1].iStart+1;
#ifdef BLOCK_SIZE
            if ( (iOldSize + size) > BLOCK_SIZE )
            {
                asSegList[iNumSegs-1].iEnd =
                    asSegList[iNumSegs-1].iStart + (BLOCK_SIZE-1);
                asSegList[iNumSegs].iStart = asSegList[iNumSegs-1].iEnd+1;
                asSegList[iNumSegs++].iEnd = iEndAddr;
                if ( iNumSegs == MAX_SEGMENTS )
                {
					printk( "Tried to load too many segments >%d\n", iNumSegs );
                    break;
                }
            }
            else
#endif
            {
                asSegList[iNumSegs-1].iEnd = iEndAddr;
            }
        }

/*----- Create new segment if it isn't adjoined ----------------------*/
        if ( bJoinedSegment == FALSE )
        {
            asSegList[iNumSegs].iStart = iStartAddr;
            asSegList[iNumSegs++].iEnd = iEndAddr;
            if ( iNumSegs == MAX_SEGMENTS )
            {
                break;
            }
        }
        bJoinedSegment = FALSE;
    }
    file_close( fsp );
    iTotalSize = 0;
    for ( i = 0; i < iNumSegs; i++ )
    {
        asSegList[i].iSize = asSegList[i].iEnd - asSegList[i].iStart+1;
        asSegList[i].iOffset = iTotalSize;
        iTotalSize += asSegList[i].iSize;
		printk( KERN_DEBUG "Segment %d : %Xh - %Xh\n", i+1,
			    asSegList[i].iStart, asSegList[i].iEnd );
    }
    iGlobalTotalSize = iTotalSize * 2;
    return ( iNumSegs );
}

void UpdateLoadProgress( int iProgress )
{
	static int iLastProgress = -1;

	if ( iLastProgress != iProgress )
	{
		iLastProgress = iProgress;
		if ( iProgress != -1 )
		{
			printk( "Progress %d%%\n", iProgress );
		}
	}
}

/*f*********************************************************************
* NAME: void ReadSegment( BYTE *data, seg_item_t *psSegInfo )
*
* DESCRIPTION:
*   Reads the data in the segment specified in the psSegInfo struct, and
*   places it at the data pointer.
*
* INPUTS:
*   data                place to store data
*   psSegInfo           struct containing segment information
*   filename            HEX file to read from
*
* OUTPUTS:
*   None.
*
* NOTES:
*   None.
*
*f*/

void ReadSegment(BYTE *data, seg_item_t *psSegInfo, const char *filename)
{
    UINT size, iStartAddr, iRecType;
    int bSegStarted = FALSE;
	BOOL bFinished = FALSE;
    int iSegDatacnt = 0;
    struct file *fsp;
    UINT temp;
    int i;
    int iAddress;
	char zLine[MAX_LINE_LENGTH+1];
	int iRet;
	char zString[16];
	int iPos;

    fsp = file_open( filename, O_RDONLY, 0 );
	while ( bFinished == FALSE )
    {
		iRet = freadline( fsp, zLine, MAX_LINE_LENGTH );
		if ( iRet <= 0 )
		{
			bFinished = TRUE;
			continue;
		}
		if ( iRet == 0 )
		{
			continue;
		}
		iPos = 1;
		strncpy( zString, &zLine[iPos], 2 );
        sscanf( zString, "%02X", &size );
		iPos += 2;
		strncpy( zString, &zLine[iPos], 4 );
        sscanf( zString, "%04X", &iStartAddr );
		iPos += 4;
		strncpy( zString, &zLine[iPos], 2 );
        sscanf( zString, "%02X", &iRecType );
		iPos += 2;
        if ( ( (iStartAddr+size-1) < psSegInfo->iStart ) && ( !bSegStarted ) )
        {
            continue;
        }
        bSegStarted = TRUE;
        for ( i = 0; i < (int)size; i++ )
        {
            iAddress = iStartAddr + i;
			strncpy( zString, &zLine[iPos], 2 );
			iPos += 2;
            sscanf( zString, "%02X", &temp );
            if ( iAddress >= psSegInfo->iStart )
            {
                data[iSegDatacnt++] = (BYTE ) temp;
            }
            if ( iSegDatacnt == psSegInfo->iSize )
            {
                break;
            }
        }
        if ( iSegDatacnt == psSegInfo->iSize )
        {
            break;
        }
    }
    file_close( fsp );
}


/*F*********************************************************************
* NAME: BOOL FileExists( char *pzFileName )
*
* DESCRIPTION:
*   Checks to see if a file exists.
*
* INPUTS:
*   pzFileName          the file to check.
*
* OUTPUTS:
*   BOOL                TRUE if file exists,
*                       FALSE otherwise.
*
* NOTES:
*
*F*/

int FileExists( char *pzFileName )
{
	int iRet = -1;
	struct file *fp;

    if ( (fp=file_open(pzFileName,O_RDWR, 0)) != NULL )
    {
		iRet = 0;
        file_close( fp );
    }
	return ( iRet );
}

/*F*********************************************************************
* NAME: BOOL SegExists( int iStart )
*
* DESCRIPTION:
*   Checks to see if a segment exists in the list of segments.
*
* INPUTS:
*   iStart          the start address of the segment to look for.
*
* OUTPUTS:
*   BOOL            TRUE if the segment exists,
*                   FALSE otherwise.
*
* NOTES:
*
*F*/

BOOL SegExists( int iNumSegments, int iStart)
{
    BOOL bExists = FALSE;
    int i;

    for ( i = 0; i < iNumSegments; i++ )
    {
        if ( asSegList[i].iStart == iStart )
        {
            bExists = TRUE;
            break;
        }
    }
    return ( bExists );
}

/*F*********************************************************************
* NAME: void DumpMemory( int iAddress, BYTE *pData, int iBytes)
*
* DESCRIPTION:
*   Dump buffer to the terminal
*
* INPUTS:
*   iAddress        start address
*   pData           buffer
*   iBytes          bytes to dump
*
* OUTPUTS:
*   None.
*
* NOTES:
*
*F*/

void DumpMemory( int iAddress, BYTE *pData, int iBytes)
{
    char zData[128] = "";
    int i;

    for ( i = 0; i < iBytes; i++ )
    {
        if ( ( ( i % 8 ) == 0 ) && ( i > 0 ) )
        {
			strcat( zData, " " );
        }
        if ( ( i % 16 ) == 0 )
        {
            if ( i != 0 )
            {
                printk( KERN_INFO "%s\n", zData );
            }
			zData[0] = 0;
            if ( iAddress != -1 )
            {
				sprintf( &zData[strlen(zData)], "%04X - ", iAddress );
            }
        }
        iAddress++;
		sprintf( &zData[strlen(zData)], "%02X ", pData[i] );
    }
    printk( KERN_INFO "%s\n", zData );
}


