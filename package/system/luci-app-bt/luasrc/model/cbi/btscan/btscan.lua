--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.tools.webadmin")
require("luci.config")

local m, s, si, sa, pt, o, le, ll, ld, lc

local nixio  = require "nixio"

m = Map("btscan", translate("Bluetooth"),
        translate("Scan for bluetooth devices"))
--        translate("Configure bluetooth logging settings"))

--s = m:section(TypedSection, "config", "")
--s.anonymous = true
--s.addremove = false

--si = s:option(Value, "site_id", translate( "Site ID" ), 
--	      translate("Sets the site ID to be sent with bluetooth data"))
--si.datatype = "uinteger"

--sa = s:option(Value, "server_address", translate( "Server Address" ),
--	      translate("Specifies the address of the UDP server"))
--sa.datatype = "ip4addr"

--pt = s:option(Value, "server_port", translate("Server Port"),
--        translate("Specifies the port of the UDP server"))
--pt.datatype = "port"

--le = s:option(Flag, "local_logging",
--        translate("Local logging"),
--        translate("Enable local logging of detected bluetooth MACs"))
--le.rmempty = false
--le.optional = false

--ll = s:option(Value, "log_dir", translate( "Log Directory" ),
--	      translate("Sets the local log directory"))
--ll.datatype = "string"
--ll:depends("local_logging", "1")

--ld = s:option(Value, "log_days", translate("Log Days"),
--        translate("The number of days log files are stored (1-30)"))
--ld.datatype = "range(1,30)"
--ld:depends("local_logging", "1")

--lc = s:option(Flag, "log_compress", translate("Compress Log Files"),
--        translate("Whether to compress old log files"))
--lc.enabled  = "1"
--lc.disabled = "0"
--lc.rmempty = false
--lc.optional = false
--lc:depends("local_logging", "1")

--Show current bluetooth state (diagnostics)
t = m:section(SimpleSection)
t.template = "admin_status/btscan"

return m

