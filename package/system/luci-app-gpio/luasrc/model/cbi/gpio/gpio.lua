-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

require("luci.model.uci")
require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.tools.webadmin")
require("luci.config")
local fs = require "nixio.fs"

local m, s, t, status, state, number, direction, operation, a

m = Map("gpio", translate("GPIO"),
	      translate(""))

s = m:section(TypedSection, "_dummy", translate("Configuration"))
s.anonymous = true

number = s:option(Value, "GPIO", translate("GPIO"))

--Read GPIO from /sys/class/gpio
local gpiochips = fs.glob("/sys/class/gpio/gpiochip*")
if gpiochips then
	local gpiochip, base, ngpio
	for gpiochip in gpiochips do
		base = nixio.fs.readfile( gpiochip .. "/base" )
		ngpio = nixio.fs.readfile( gpiochip .. "/ngpio" )

		for ii = tonumber(base),(tonumber(base)+tonumber(ngpio)-1) do number:value(ii) end
	end
end

operation = s:option(ListValue, "operation", translate("Operation"))
operation:value("export")
operation:value("unexport")

direction = s:option(ListValue, "direction", translate("Direction"))
direction:value("in")
direction:value("out")
direction:depends("operation", "export")

state = s:option(Flag, "state", translate("State"))
state.rmempty = false
state:depends("direction", "out")

p_switch = s:option(Button, "_switch")
p_switch.title      = translate("Set")
p_switch.inputtitle = translate("Set")
p_switch.inputstyle = "apply"

--Show current GPIO state (diagnostics)
t = m:section(SimpleSection)
t.template = "admin_status/gpio"

function s.cfgsections()
	return { "_number" }
end

--Perform export/unexport
if p_switch:formvalue("_number") then
	local gpio_number = number:formvalue("_number")
	local gpio_direction = direction:formvalue("_number")
	local gpio_state = state:formvalue("_number")
	local gpio_operation = operation:formvalue("_number")
	local filename

	if gpio_operation == "unexport" then
		os.execute( "echo " .. gpio_number .. " >/sys/class/gpio/unexport" )
	else
		filename = "/sys/class/gpio/gpio" .. gpio_number
		if not nixio.fs.access(filename) then
			os.execute( "echo " .. gpio_number .. " >/sys/class/gpio/export" )
		end
		os.execute( "echo " .. gpio_direction .. " >/sys/class/gpio/gpio" .. gpio_number .. "/direction" )
		if gpio_state then
			os.execute( "echo " .. gpio_state .. " >/sys/class/gpio/gpio" .. gpio_number .. "/value" )
		end
	end
end

return m

