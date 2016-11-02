#!/bin/sh /etc/rc.common
### BEGIN INIT INFO
# Provides:          gpioluasetup
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     S
# Default-Stop:      
# Short-Description: Provide named links for GPIO access
# Description:       Provide named links for GPIO access. Desired to be in /sys but might not be possible.
### END INIT INFO

START=20

USE_PROCD=1
NAME=gpio


validate_section_gpio()
{
        uci_validate_section gpio gpio "${1}" \
                'state:bool:1' \
                'direction:string' \
				'syslabel:string'
        return $?
}

gpio_instance()
{
	validate_section_gpio "${1}" || {
		echo "Validation failed"
		return 1	
	}
	NUMBER=`echo ${1} | cut -c5-10`
	if [ ! -d /sys/class/gpio/$syslabel ]; then
		echo $NUMBER > /sys/class/gpio/export
		echo ${direction} >/sys/class/gpio/$syslabel/direction
		if [ ${direction} == "out" ]; then
			echo ${state} > /sys/class/gpio/$syslabel/value
		fi
	fi
}

start_service()
{
	if [ ! -f /etc/config/gpio ]; then
		touch /etc/config/gpio
	fi
    config_load "${NAME}"
	config_foreach gpio_instance interface
	exit 0
}

stop_service()
{
	# Any todo gpio off
	exit 0
}

