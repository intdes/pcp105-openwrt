#ifndef hyrax_driver_h
#define hyrax_driver_h

#include <linux/gpio.h>

/*----- context ------------------------------------------------------*/

typedef unsigned char UCHAR;
typedef int BOOL;
typedef unsigned char U8;
typedef unsigned short UU16;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;

#define DEVICE_NAME						"pcp105mcu"

#define FALSE							0
#define TRUE							1

#define LO8(x)							((x)&0x00FF)
#define HI8(x)							(((x)>>8)&0x00FF)
#define MIN(x,y)						(((x)<(y))?(x):(y))
#define TOBOOL(x)						((((x)!=FALSE))?TRUE:FALSE)
#define NOT(x)							((((x)==TRUE))?FALSE:TRUE)

#ifdef false
#undef false
#endif
#ifdef true
#undef true
#endif
#define false				FALSE
#define true				TRUE

enum PH_STATUS {
	FILTER_MATCH = 0x80,
	FILTER_MISS = 0x40,
	PACKET_SENT = 0x20,
	PACKET_RX = 0x10,
	CRC_ERROR = 0x08,
	TX_FIFO_ALMOST_EMPTY = 0x02,
	RX_FIFO_ALMOST_FULL = 0x01
};

#define DEFAULT_TIMEOUT					1000
#define MAX_RF_BUFFER_SIZE				255
#define MAX_HEADER_SIZE					4
#define SPI_TRANSFER_BUF_LENGTH			(MAX_RF_BUFFER_SIZE+MAX_HEADER_SIZE)
#define DEFAULT_PACKET_SIZE				120

#define MAX_FILENAME_LENGTH				256

struct hyrax_priv {
	struct gpio_chip gc;
    struct i2c_client *i2c;
	struct watchdog_device wdt;
	struct cdev cdev;
	char zFileName[MAX_FILENAME_LENGTH+1];
	int iRevision;
    uint16_t irq_mask;
    uint16_t irq_stat;
    uint16_t irq_trig_raise;
    uint16_t irq_trig_fall;
    int irq_base;
    struct mutex irq_lock;
	struct irq_domain *irq_domain;
};

/*----- prototypes ---------------------------------------------------*/

//hyrax_fs
int hyrax_charger_init_sysfs( struct i2c_client *i2c );
void hyrax_charger_destroy_sysfs( struct i2c_client *i2c );

//hyrax_file
struct file* file_open(const char* path, int flags, int rights);
void file_close(struct file* file);
int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size);
int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size);
int file_sync(struct file* file);

//hyrax_watchdog
int hyrax_init_wdt( struct i2c_client *i2c );
void hyrax_close_wdt( struct i2c_client *i2c );

/*----- variables ----------------------------------------------------*/

#ifdef GLOBAL
const char *dev_info = DEVICE_NAME;
#else
extern const char *dev_info;
#endif

#endif

