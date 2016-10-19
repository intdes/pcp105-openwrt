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

LINKS_DIR=/var/etc/gpiolinks/
HOSTTYPE=/var/etc/hosttype

start_service()
{
	do_hosttype
	do_platform_init	
	do_makelinks
	do_setgpios
	exit 0
}

stop_service()
{
	# Any todo gpio off
	exit 0
}

get_version()
{
	PROTOCOL=`i2cget -f -y 0 0x50 0x7f b | cut -d'x' -f2`
	if [[ "$PROTOCOL" == "ff" ]] ; then
		VERSION="UNKNOWNP1"
		R_PCA_C="0"
		VER_ENC="0"
	else
    	R_PCA_C=`i2cget -f -y 0 0x50 0x11 b | cut -d'x' -f2`
	    R_PCA_C=$R_PCA_C`i2cget -f -y 0 0x50 0x12 b | cut -d'x' -f2`
    	R_PCA_C=$R_PCA_C`i2cget -f -y 0 0x50 0x13 b | cut -d'x' -f2`
		if [[ "$R_PCA_C" == "0068c3" ]]; then
			VERSION="PCP105"
		elif [[ "$R_PCA_C" == "007097" ]]; then
			VERSION="FIT"
		else
			VERSION="UNKNOWN"
		fi

		VER_ENC=`i2cget -f -y 0 0x50 0x14 b | cut -d'x' -f2`
		if [[ "$VER_ENC" == "91" ]] ; then
			VERSION=$VERSION"P1"
		elif [[ "$VER_ENC" == "92" ]] ; then
			VERSION=$VERSION"P2"
		elif [[ "$VER_ENC" == "93" ]] ; then
			VERSION=$VERSION"P3"
		elif [[ "$VER_ENC" == "94" ]] ; then
			VERSION=$VERSION"P4"
		elif [[ "$VER_ENC" == "a0" ]] ; then
			VERSION=$VERSION"A"
		else
			#Set hw rev if an unknown value is detected
			if [[ "$VERSION" == "PCP105" ]]; then
				VERSION=$VERSION"P2"
			elif [[ "$VERSION" == "FIT" ]]; then
				VERSION=$VERSION"P1"
			else
				VERSION=$VERSION"P1"
			fi
		fi
	fi
}

waitfor()
{	
	FILE=$1
	READY=0
	TIMEOUT=5
	while [ $READY -eq 0 ]; do
		if [ -e $FILE ]; then
			READY=1
			continue
		fi
		sleep 1
		TIMEOUT=$(( $TIMEOUT - 1 ))
		if [ $TIMEOUT -eq 0 ]; then
			break
		fi
	done
}

do_hosttype()
{
	insmod /lib/modules/4.4.7/i2c-designware-core.ko
	insmod /lib/modules/4.4.7/i2c-designware-platform.ko
	waitfor /dev/i2c-0

	echo "-Setting host type..."
	# set unit type
	get_version
	HOSTNAME=$VERSION
	mkdir -p /var/etc
	echo -n $HOSTNAME > $HOSTTYPE

	rmmod i2c-designware-platform
	rmmod i2c-designware-core
}

do_platform_init()
{
	insmod /lib/modules/4.4.7/pcp_i2c_init.ko iPCARevision=0x$VER_ENC iPCAId=0x$R_PCA_C
	insmod /lib/modules/4.4.7/i2c-designware-core.ko
	insmod /lib/modules/4.4.7/i2c-designware-platform.ko
}

do_setgpios()
{
	echo "-Setting gpios..."
	# get hosttype
	HOSTNAME="$(cat $HOSTTYPE)"

	case $HOSTNAME in
		PCP105P1)
			;;

		PCP105P2)
			;;
	esac
}

do_makelinks()
{
	echo "-Making links..."
	# get hosttype
	HOSTNAME="$(cat $HOSTTYPE)"
	mkdir -p $LINKS_DIR

	case $HOSTNAME in
		PCP105P1)
			;;

		PCP105P2)
			;;
	esac

}

