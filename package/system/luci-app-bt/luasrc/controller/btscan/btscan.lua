-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.btscan.btscan", package.seeall)

local io = require "io"

function index()
	entry({"admin", "network", "btscan"}, cbi("btscan/btscan"), _("Bluetooth"), 59)
	entry({"admin", "network", "btscan", "scan"}, call("_scan") ).leaf = true
end

function exec(command)
    local pp   = io.popen(command)
    local data = pp:read("*a")
    pp:close()

    return data
end

-- execi
--
-- Executes a command, returns an iterator that'll step through the output line by line
function io.execi(command)
    local fh = io.popen( command, "r" )
    assert(fh, "Failed to invoke asterisk")

    return function()
        local ln = fh:read("*l")
        if not ln then 
			fh:close() 
		end
        return ln
    end
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

-- _scan
--
-- Performs a bluetooth scan 
function _scan()
	local data
	local rv = {}
	local entries = 0
	local array

	data = exec( "hciconfig hci0" )
	if data:find( "DOWN" ) then
		exec( "hciconfig hci0 up" )
	end
	exec( "hciconfig hci0 inqmode 1" )

	for line in io.execi("/usr/local/bin/inq -s") do
		array = split( line, "," )
		rv[entries+1] = {
			mac = array[0],
			name = array[1],
			rssi = array[2]
		}
		entries = entries + 1
	end
    luci.http.prepare_content("application/json")
    luci.http.write_json(rv)
end

