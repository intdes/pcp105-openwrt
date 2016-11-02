-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.gpio.gpio", package.seeall)

function index()
	entry({"admin", "system", "gpio"}, cbi("gpio/gpio", {hideapplybtn=true, hidesavebtn=true, hideresetbtn=true}), _("GPIO Configuration"), 59)
	entry({"admin", "system", "gpio", "remove"}, call("_remove") ).leaf = true
	entry({"admin", "system", "gpio", "toggle"}, call("_toggle") ).leaf = true
	entry({"admin", "system", "gpio", "poll"}, call("_poll") ).leaf = true
end

local uci  = require "luci.model.uci".cursor()
local fs = require "nixio.fs"

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

-- _poll
--
-- Called from gpio.htm, polls the gpio state 
function _poll()
	local rv = {}
	local sys_gpios = fs.glob("/sys/class/gpio/*")
	local sys_gpio_instance
	local gpio_number, gpio_label = nil
	local gpio_port_number
	local entries = 0

	for sys_gpio_instance in sys_gpios do
		if sys_gpio_instance:find("export") then
			--do nothing
		elseif sys_gpio_instance:find("gpiochip") then
			--do nothing
		elseif sys_gpio_instance:find("pio") then

			gpio_port_number = sys_gpio_instance:sub( 17 )
			if gpio_port_number:sub(1,4) == "gpio" then
				gpio_number = gpio_port_number:sub( 5 )
			elseif gpio_port_number:sub(1,3) == "pio" then
				gpio_number = label_to_number( gpio_port_number )
			end


			--Read gpio label from config or /etc/gpiolinks
			local gpios = fs.glob("/etc/gpiolinks/*")
			local gpio_instance

			gpio_label = uci:get("gpio", "gpio" .. gpio_number, "label")

			if not gpio_label then
				for gpio_instance in gpios do
					gpio_link = nixio.fs.readlink( gpio_instance )
					if gpio_link == "/sys/class/gpio/" .. gpio_port_number then
						gpio_label = gpio_instance:sub(16)
						break
					end
				end
			end

			rv[entries+1] = {
				gpio = gpio_number,
				value = nixio.fs.readfile("/sys/class/gpio/" .. gpio_port_number .. "/value"),
				direction = nixio.fs.readfile("/sys/class/gpio/" .. gpio_port_number .. "/direction"),
				label = gpio_label,
				gpio_port_number = gpio_port_number
			
			}
			entries = entries + 1
		end
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end

local fs = require "nixio.fs"
local uci  = require "luci.model.uci".cursor()

-- save_settings
--
-- Save settings to a file, duplicated from module code
function save_settings()

       
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

			--read labels from config file
			label = uci:get("gpio", "gpio" .. number, "label")
			if ( label ) then
				buffer = buffer .."\toption label '" .. label .. "'\n"
			end

			if direction == "out" then
				nixio.syslog( "info", "reading state from /sys/class/gpio/" .. gpio_port_number .. "/value" )
				state = nixio.fs.readfile("/sys/class/gpio/" .. gpio_port_number .. "/value")
				state = state:gsub(" *\n", "")
				nixio.syslog( "info", "value: " .. state )
				buffer = buffer .."\toption state '" .. state .. "'\n"
			end
			buffer = buffer .."\n"
		end
	end
	fs.writefile( "/etc/config/gpio", buffer )
end

-- _remove
--
-- Unexports a GPIO
function _remove(index)
	os.execute( "echo " .. luci.http.formvalue("gpio" ) .. " >/sys/class/gpio/unexport" )
	save_settings()
end

-- _toggle
--
-- Toggles a GPIO
function _toggle(index)
	os.execute( "echo " .. luci.http.formvalue("value" ) .. " >/sys/class/gpio/" .. luci.http.formvalue("gpio") .. "/value" )
	save_settings()
end

