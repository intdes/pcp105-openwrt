#!/bin/sh
if [ ! $# -eq 1 ]; then
	echo usage: update_firmware.sh [full/path/to/firmware/file.hex]
	exit
fi
LATEST_FIRMWARE=$1
FILE_REVISION_STRING=`echo $LATEST_FIRMWARE | sed -n 's/.*v\([0-9]*\.[0-9]*\).*/\1/p'`
if [ "z"$FILE_REVISION_STRING == "z" ]; then
	echo invalid file format
	exit
fi

STATE=`cat /sys/devices/pci0000:00/0000:00:15.2/i2c_designware.0/i2c-0/0-0066/state`

if [ ! -f ${LATEST_FIRMWARE} ]; then
    echo "Firmware not found"
elif [ ! "z"${STATE} == "z1" -a ! "z"${STATE} == "z2" ]; then
    echo "MCU is in invalid state ${STATE}"
else
    MCU_STRING=`cat /sys/devices/pci0000:00/0000:00:15.2/i2c_designware.0/i2c-0/0-0066/revision`
    MAJOR=`echo ${MCU_STRING} | cut -d. -f1`
    MINOR=`echo ${MCU_STRING} | cut -d. -f2`
    MCU_REVISION=$(( $MAJOR * 100 + $MINOR ))

    MAJOR=`echo ${FILE_REVISION_STRING} | cut -d. -f1`
    MINOR=`echo ${FILE_REVISION_STRING} | cut -d. -f2`
    LATEST_REVISION=$(( $MAJOR * 100 + $MINOR ))

    UPGRADE=$(( $LATEST_REVISION > $MCU_REVISION ))
    if [ ${UPGRADE} -eq 1 ]; then
		echo Performing upgrade...
        echo ${LATEST_FIRMWARE} >/sys/devices/pci0000:00/0000:00:15.2/i2c_designware.0/i2c-0/0-0066/upload
		echo Upgrade complete.
	else
		echo Skipping upgrade, local firmware is up to date [$MCU_STRING]
    fi
fi

