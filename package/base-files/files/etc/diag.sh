#!/bin/sh
# Copyright (C) 2006-2009 OpenWrt.org

. /lib/functions/leds.sh
. /usr/local/bin/common.shlib

get_status_led() {
	get_hw_info

	RET=0
	FLASH=0
	case "$HWINFO" in
	PCP105P1)
		RET=1
		;;
	PCP105*)
		status_led="heartbeat_led"
		FLASH=1
		;;
	FIT*)
		status_led="heartbeat_blue"
		;;
	NEC*)
		status_led="heartbeat_blue"
		;;
	*)
		RET=1
		;;
	esac
}

set_state() {
	get_status_led
	if [ $? -eq 0 ]; then
		case "$1" in
		preinit)
			status_led_blink_preinit
			;;
		failsafe)
			status_led_blink_failsafe
			;;
		done)
			if [ $FLASH -eq 1 ]; then
				status_led_set_heartbeat
			else
				status_led_on
			fi
			;;
		esac
	fi
}

