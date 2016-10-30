#!/bin/sh /etc/rc.common
### BEGIN INIT INFO
# Provides:          setmac
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     S
# Default-Stop:
# Short-Description: Sets the MAC address.
# Description:       Sets the MAC address based on EEPROM settings.
### END INIT INFO

START=18
USE_PROCD=1

UNSET_MAC="ff:ff:ff:ff:ff:ff"
DEV_BUS=0

#reads the eth0 MAC address
read_e0_mac()
{
	R_E0_MAC=`i2cget -f -y $DEV_BUS 0x50 0x21 b | cut -d'x' -f2`":"
	R_E0_MAC=$R_E0_MAC`i2cget -f -y $DEV_BUS 0x50 0x22 b | cut -d'x' -f2`":"
	R_E0_MAC=$R_E0_MAC`i2cget -f -y $DEV_BUS 0x50 0x23 b | cut -d'x' -f2`":"
	R_E0_MAC=$R_E0_MAC`i2cget -f -y $DEV_BUS 0x50 0x24 b | cut -d'x' -f2`":"
	R_E0_MAC=$R_E0_MAC`i2cget -f -y $DEV_BUS 0x50 0x25 b | cut -d'x' -f2`":"
	R_E0_MAC=$R_E0_MAC`i2cget -f -y $DEV_BUS 0x50 0x26 b | cut -d'x' -f2`
}

### END INIT INFO
# Function that starts the daemon/service
#
start_service()
{
	read_e0_mac
	if [ ! $R_E0_MAC == $UNSET_MAC ]; then
		ifconfig eth0 hw ether $R_E0_MAC
	fi
}

stop_service()
{
	echo
}

