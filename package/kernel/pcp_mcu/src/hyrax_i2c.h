#ifndef hyrax_i2c_h
#define hyrax_i2c_h

// I2C Command Register Definitions
#define IC_REG_BUILD_NUM_MAJOR	0x01
#define IC_REG_BUILD_NUM_MINOR	0x02

#define IC_WATCHDOG_INIT			0x03
#define IC_WATCHDOG_KICK			0x04

#define IC_REG_DEVICE_STATE			0x80
#define IC_REG_BOOT_READ_DATA		0x81
#define IC_REG_BOOT_WRITE_DATA		0x82
#define IC_REG_BOOT_PACKET_LENGTH	0x83
#define IC_REG_BOOT_BASE_ADDRESS	0x84
#define IC_REG_BOOT_CRC				0x85
#define IC_REG_BOOT_RESET_MAP		0x86
#define IC_REG_BOOT_GET_BASE_ADDRESS	0x87

int GetRevision(struct device *dev );
int LoadFirmware( struct device *dev, char *pzFileName );
int SetState( struct device *dev, int iState );
int GetState( struct device *dev );
int SetWatchdogTimeout( struct device *dev, WORD iTimeout );
int KickWatchdog( struct device *dev );

#endif	//hyrax_i2c_h

