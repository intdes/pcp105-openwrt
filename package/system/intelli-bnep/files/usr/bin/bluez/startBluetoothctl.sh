#!/bin/ash
bluetoothctl << EOF
power on
discoverable on
agent on
quit
EOF
