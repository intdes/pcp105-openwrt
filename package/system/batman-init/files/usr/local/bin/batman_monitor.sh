#!/bin/sh

#
# scan -
#
# Check connection to neighbouring nodes
#
scan()
{
        #Dont check if batman isnt running
        if [ ! -e /sys/kernel/debug/batman_adv/bat0 ]; then
                return 0
        fi

        NODES=`batctl tg | wc -l`
        NODES=$(( $NODES - 2 ))

        SCAN_RESULT=0
        if [ $NODES -gt 0 ]; then
                SCAN_RESULT=1
        fi
        while [ $NODES -gt 0 ]; do
                NODE=`batctl tg | tail -n $NODES | head -n 1 | cut -d" " -f3`

                batctl p -c1 $NODE >/dev/null
                if [ $? -eq 0 ]; then
                        SCAN_RESULT=0
                        break
                fi

                NODES=$(( $NODES - 1 ))
        done
        return $SCAN_RESULT
}

#
# restart_wifi -
#
# Stop wifi interface, triggering a restart of wifi and batman in hotplug.
#
restart_wifi()
{
        wifi down
        #wifi will be restated by hotplug
}


DELAY_BETWEEN_SCANS=15
FAILURE_THRESHOLD=4
SCANNING=1
FAILURES=0

# Periodically check status of BATMAN network, and restart if communications fail
while [ $SCANNING -eq 1 ]; do
        scan
        RESULT=$?
        if [ ! $RESULT -eq 0 ]; then
                FAILURES=$(( $FAILURES + 1 ))
                if [ $FAILURES -ge $FAILURE_THRESHOLD ]; then
                        logger -t BATMAN "Restarting wifi/BATMAN after connection dropout"
                        restart_wifi
                        FAILURES=0
                fi
        else
                FAILURES=0
        fi
        sleep $DELAY_BETWEEN_SCANS
done

