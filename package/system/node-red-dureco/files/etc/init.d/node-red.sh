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
		#Update landing page if necessary
		uci show uhttpd.main.index_page
		if [ ! $? -eq 0 ]; then 
	        uci set uhttpd.main.index_page=landing.html
	        uci commit
			sync
	        /etc/init.d/uhttpd restart
		fi

		gpspipe -r | nc -u 127.0.0.1 5111 &

        export HOME="/root"
        node-red < /dev/null 2>&1 >/dev/null &

		#Add cron monitoring task
	    test -f /etc/crontabs/root || touch /etc/crontabs/root
	    grep -q 'check_node' /etc/crontabs/root || {
    	    echo "* * * * *   /usr/local/bin/check_node.sh" >> /etc/crontabs/root
	    }
		sync

	    /etc/init.d/cron restart &

        exit 0
}

stop_service()
{
        killall node-red
        exit 0
}

