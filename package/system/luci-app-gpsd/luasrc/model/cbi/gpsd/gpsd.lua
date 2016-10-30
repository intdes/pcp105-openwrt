-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

require("luci.model.uci")
require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.tools.webadmin")
require("luci.config")

local m, s, si, device, pps_device, status, port, listen, enabled

m = Map("gpsd", translate("GPSd"),
	      translate(""))

function m.on_commit(map)
    luci.util.exec("/etc/init.d/gpsd restart");
end


s = m:section(NamedSection, "core", "gpsd", translate("Settings") )
s.anonymous = true
s.addremove = false

device = s:option(Value, "device", translate("Device"))
device.rmempty = false

pps_device = s:option(Value, "pps_device", translate("PPS Device"))
pps_device.rmempty = false

port = s:option(Value, "port", translate("Port"))
port.rmempty = false

listen = s:option(Flag, "listen_globally",
		        translate("Listen Globally") )
listen.rmempty = false
listen.optional = false

enabled = s:option(Flag, "enabled",
		        translate("Enabled") )
enabled.rmempty = false
enabled.optional = false

local device_suggestions = nixio.fs.glob("/dev/tty[A-Z]*")
    or nixio.fs.glob("/dev/tts/*")

if device_suggestions then
    local node
    for node in device_suggestions do
        device:value(node)
    end
end

status = s:option(Value, "device", translate("Device"))
status.template = "admin_status/gpsd"

return m

