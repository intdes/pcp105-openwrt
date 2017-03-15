--- IntelliDesign Pty Ltd
--- Copyright 2017 - All rights reserved
--- Custom batman-adv lua script file
--- version 1.0

local m = Map("batman-adv",
	translate("Batman-Adv"),
	translate("Better Approach To Mobile Adhoc Networking"))
local section = m:section(TypedSection, "mesh", translate("Settings"))

local interface = section:option(Value, "mesh_iface", translate("Add to interface"))
interface.widget = "radio"
interface.template  = "cbi/network_ifacelist"
interface.nobridges = true

local ip = section:option(Value, "ip", translate("Node IP:"))
ip.datatype = "list(neg(or(uciname,hostname,ip4addr)))"
ip.placeholder = "0.0.0.0"
local mask = section:option(Value, "mask", translate("Netmask"))
mask.datatype = "list(neg(or(uciname,hostname,ip4addr)))"
mask.placeholder = "0.0.0.0"

local bw = section:option(Value, "gw_bandwidth", translate("Gateway bandwidth"),
    translate("10000 => 10.0 Up/2.0 Down MBit, 5000 => 5.0 Up/ 1.0 Down MBit"))
bw.datatype = "uinteger"

interval = section:option(Value, "orig_interval", translate("Originator interval"))
interval.datatype = "uinteger"

iso = section:option(Value, "isolation_mark", translate("Isolation mark"))

hop = section:option(Value, "hop_penalty", translate("Hop penalty"))
hop.datatype = "uinteger"

section:option(Flag, "ap_isolation", translate("Enable ap-isolation")).rmempty = false
section:option(Flag, "fragmentation", translate("Fragmentation")).rmempty = false

section:option(Flag, "bonding", translate("Bonding mode")).rmempty = false
section:option(Flag, 'bridge_loop_avoidance', translate("Bridge loop avoidance")).rmempty = false
section:option(Flag, "aggregated_ogms", translate("Aggregated original messages")).rmempty = false

local g = section:option(ListValue, "gw_mode", translate("Gatewate mode"))
g:value("server", translate("server mode"))
g:value("client", translate("client mode"))
g:value("off", translate("off"))

local p = section:option(ListValue, "routing_algo", translate("Routing algorithm"))
p:value("BATMAN_V", translate("BATMAN V"))
p:value("BATMAN_IV", translate("BATMAN IV"))

function m.on_commit(any)
	luci.util.exec('/etc/init.d/batman-adv restart')
end

return m



