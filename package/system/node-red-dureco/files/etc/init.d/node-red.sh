#!/bin/sh /etc/rc.common
# chkconfig: S 10 10
# description: Starts the bnep service daemon
#
### BEGIN INIT INFO
# Provides:          node-red startup script
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

start_service()
{
        export HOME="/root"
        node-red < /dev/null 2>&1 >/dev/null &
        exit 0
}

stop_service()
{
        killall node-red
        exit 0
}
