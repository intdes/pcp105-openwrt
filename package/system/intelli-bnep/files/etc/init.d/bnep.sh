#!/bin/sh /etc/rc.common
# chkconfig: S 10 10
# description: Starts the bnep service daemon
#
### BEGIN INIT INFO
# Provides:          bt-pan daemon
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description:
# Description:
#

START=99
USE_PROCD=1

### END INIT INFO
# Function that starts the daemon/service
#
get_process_id()
{
    ps | grep $1 | grep -qv grep
	if [ $? -eq 0 ]; then
		RET_PID=`ps | grep $1 | grep -v grep | awk '{print $1}'`
	else
		RET_PID=-1
    fi
} 

start_service()
{
		hciconfig hci0 lm master,accept
		hciconfig hci0 up

		sleep 1

		brctl addbr bnep
		brctl setfd bnep 0
		brctl stp bnep off
		ip addr add 192.168.2.2/24 dev bnep
		ip link set bnep up

		sleep 1

		/etc/init.d/network restart

		sleep 1

		/usr/bin/bluez/startBluetoothctl.sh > /dev/null
		/usr/bin/bluez/bt-pan --trustall --discover_time 0 -i hci0 server bnep &
        exit 0
}
:
stop_service()
{
		get_process_id bt-pan
		kill -SIGINT $RET_PID	
		exit 0
}
