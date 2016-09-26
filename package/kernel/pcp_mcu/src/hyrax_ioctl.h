#ifndef v2v_ioctl_h
#define v2v_ioctl_h

typedef struct 
{
	unsigned char iGroup;
	unsigned char iStart;
	unsigned char iValue;
} prop_t;

#define V2V_IOCTL                  	0x82

#define IOCTL_SET_PROP              _IOW(V2V_IOCTL, 0, prop_t)
#define IOCTL_GET_PROP              _IOWR(V2V_IOCTL, 1, prop_t)
#define IOCTL_FLUSH_BUFFER			_IO(V2V_IOCTL, 2)
#define IOCTL_GET_BUFFER_LENGTH     _IOR(V2V_IOCTL, 3, int)
#define IOCTL_GET_CRC_ERRORS	    _IOR(V2V_IOCTL, 4, unsigned long)

#endif	//v2v_ioctl_h

