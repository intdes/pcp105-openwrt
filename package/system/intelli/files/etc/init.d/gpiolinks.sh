#!/bin/sh /etc/rc.common
### BEGIN INIT INFO
# Provides:          gpiolinks
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     S
# Default-Stop:      
# Short-Description: Provide named links for GPIO access
# Description:       Provide named links for GPIO access. Desired to be in /sys but might not be possible.
### END INIT INFO

START=15

USE_PROCD=1

. /usr/local/bin/common.shlib

LINKS_DIR=/var/etc/gpiolinks/
HOSTTYPE=/var/etc/hosttype

start_service()
{
	do_hosttype
	do_makelinks
	do_setgpios
	exit 0
}

stop_service()
{
	# Any todo gpio off
	exit 0
}

do_hosttype()
{
	echo "-Setting host type..."
	# set unit type
	get_hw_info
	get_version
	mkdir -p /var/etc
	echo -n $HWINFO > $HOSTTYPE
}

do_setgpios()
{
	echo "-Setting gpios..."

	case $HWINFO in
		PCP105P1)
			;;

		PCP105P2)
			;;

		FITP1)
			;;

		NECP1)
			;;

	esac
}

do_makelinks()
{
	echo "-Making links..."
	mkdir -p $LINKS_DIR

    case "$HWINFO" in

		PCP105P1)
			;;

		PCP105P2)
			;;
	esac

}

