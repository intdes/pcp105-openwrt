#!/bin/sh

#Start node-red if it isn't running
ps | grep node-red | grep -v grep
if [ ! $? -eq 0 ]; then
	/etc/init.d/node-red.sh start
	logger -t node-red node-red crashed, restarting
fi

#Start gpspipe if it isn't running
ps | grep gpspipe | grep -v grep
if [ ! $? -eq 0 ]; then
	gpspipe -r | nc -u 127.0.0.1 5111 &
	logger -t node-red gpspipe stopped, restarting
fi


