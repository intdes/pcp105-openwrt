-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

require("luci.model.uci")
require("luci.sys")
require("luci.sys.zoneinfo")
require("luci.tools.webadmin")
require("luci.config")
local fs = require "nixio.fs"
local uci  = require "luci.model.uci".cursor()

local m, s, t, status, state, number, direction, operation, a, label

local write_config=1

m = Map("gpio", translate("GPIO"),
	      translate(""))

s = m:section(TypedSection, "_dummy", translate("Configuration"))
s.anonymous = true

number = s:option(ListValue, "GPIO", translate("GPIO"))

operation = s:option(ListValue, "operation", translate("Operation"))
operation:value("export")
operation:value("unexport")

direction = s:option(ListValue, "direction", translate("Direction"))
direction:value("in")
direction:value("out")
direction:depends("operation", "export")

label = s:option(Value, "label", translate("Label"))
label:depends("operation", "export")

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

-- number_to_label_at91
--
-- Convert a gpio number to the sysfs label for AT91 processors
function number_to_label_at91( gpio_number )
	local numberval = tonumber(gpio_number)	
    local offset = 0
	local label

	if ( numberval ) then
		offset = numberval % 32
	else
		numberval = 0
	end
	if numberval < 32 then
		label = "pioA"
	elseif numberval < 64 then
		label = "pioB"
	elseif numberval < 96 then
		label = "pioC"
	elseif numberval < 128 then
		label = "pioD"
	else
		label = "pioE"
	end		

	label = label .. offset
	
    return label
end

-- number_to_label_generic
--
-- Convert a gpio number to the sysfs label for generic processors
function number_to_label_generic( gpio_number )
	return "gpio" .. gpio_number
end

-- is_at91
--
-- Checks if the processor is an at91 or not.
function is_at91()
	local gpio_controller
	local is_at91 = 0

	gpio_controller = nixio.fs.readfile("/sys/class/gpio/gpiochip0/label")
	gpio_controller = gpio_controller:gsub(" *\n", "")
	if gpio_controller == "fffff200.gpio" then
		is_at91 = 1
	end
	return is_at91
end

-- number_to_label
--
-- Converts a gpio number to a syfs label
function number_to_label( gpio_number )
	local label

	if is_at91() == 1 then
		label = number_to_label_at91(gpio_number)
	else
		label = number_to_label_generic(gpio_number)
	end
	return label
end

-- label_to_number
--
-- Converts a sysfs label to a gpio number
function label_to_number( label )

    local number
    local offset
    local base = 0

    if label:sub(0,4) == "pioA" then
        base = 0
    elseif label:sub(0,4) == "pioB" then
        base = 32
    elseif label:sub(0,4) == "pioC" then
        base = 64
    elseif label:sub(0,4) == "pioD" then
        base = 96
    elseif label:sub(0,4) == "pioE" then
        base = 128
    end

    offset = label:sub(5)
    number = base + offset

    return number
end

-- split
--
-- Split string using a delimeter
function split(inputstr, sep)
        if sep == nil then
                sep = "%s"
        end
        local t={} ; i=0
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                t[i] = str
                i = i + 1
        end
        return t
end

-- save_settings
--
-- Save settings to a file
function save_settings( from_ui )
	local filename
	local array
	local gpio_number, gpio_direction, gpio_state, gpio_operation, gpio_label
	local sys_label

	if from_ui == 1 then
		array = split(number:formvalue("_number"), " ")
		gpio_number = array[0]
		gpio_direction = direction:formvalue("_number")
		gpio_state = state:formvalue("_number")
		gpio_operation = operation:formvalue("_number")
		gpio_label = label:formvalue("_number")

		if gpio_operation == "unexport" then
			--unexport
			os.execute( "echo " .. gpio_number .. " >/sys/class/gpio/unexport" )
		else
			--export and write new settings to /sys/class/gpio
			sys_label = number_to_label(gpio_number)
			filename = "/sys/class/gpio/" .. sys_label 
			if not nixio.fs.access(filename) then
				os.execute( "echo " .. gpio_number .. " >/sys/class/gpio/export" )
			end
			os.execute( "echo " .. gpio_direction .. " >/sys/class/gpio/" .. sys_label .. "/direction" )
			if gpio_state then
				os.execute( "echo " .. gpio_state .. " >/sys/class/gpio/" .. sys_label .. "/value" )
			end
		end
	end

	--Write GPIO configuration to a file
	m.apply_needed = false
	m.save = false
	if write_config == 1 then
		local gpios = fs.glob("/sys/class/gpio/*")
		local gpio_instance, direction, state, label, number
		local gpio_port_number
		local buffer = ""

		for gpio_instance in gpios do
			if gpio_instance:find("export") then
				--do nothing
			elseif gpio_instance:find("gpiochip") then
				--do nothing
			elseif gpio_instance:find("pio") then

				gpio_port_number = gpio_instance:sub( 17 )
				if gpio_port_number:sub(1,4) == "gpio" then
					number = gpio_port_number:sub( 5 )
				elseif gpio_port_number:sub(1,3) == "pio" then
					number = label_to_number( gpio_port_number )
				end
				
				direction = nixio.fs.readfile("/sys/class/gpio/" .. gpio_port_number .. "/direction")
				direction = direction:gsub(" *\n", "")
				buffer = buffer .."config interface 'gpio" .. number .. "'\n"
				buffer = buffer .."\toption direction '" .. direction .. "'\n"
				buffer = buffer .."\toption syslabel '" .. gpio_port_number .. "'\n"

				--read labels from entry box and config file
				if ( from_ui == 1 and tonumber(number) == tonumber(gpio_number) ) then
					buffer = buffer .."\toption label '" .. gpio_label .. "'\n"
				else
					label = uci:get("gpio", "gpio" .. number, "label")
					if ( label ) then
						buffer = buffer .."\toption label '" .. label .. "'\n"
					end
				end

				if direction == "out" then
					state = nixio.fs.readfile("/sys/class/gpio/" .. gpio_port_number .. "/value")
					state = state:gsub(" *\n", "")
					buffer = buffer .."\toption state '" .. state .. "'\n"
				end
				buffer = buffer .."\n"
			end
		end
		fs.writefile( "/etc/config/gpio", buffer )
	end
end

--Perform export/unexport
if p_switch:formvalue("_number") then
	save_settings(1)
end


--Read GPIO from /sys/class/gpio
local gpiochips = fs.glob("/sys/class/gpio/gpiochip*")
if gpiochips then
	local gpiochip, base, ngpio
	for gpiochip in gpiochips do
		base = nixio.fs.readfile( gpiochip .. "/base" )
		ngpio = nixio.fs.readfile( gpiochip .. "/ngpio" )

		for ii = tonumber(base),(tonumber(base)+tonumber(ngpio)-1) do number:value(ii .. " (" .. number_to_label(ii) .. ")") end
	end
end

return m

