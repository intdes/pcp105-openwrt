-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.gpsd.gpsd", package.seeall)

function index()
	local page

	page = entry({"admin", "services", "gpsd"}, cbi("gpsd/gpsd"), _("GPSd"))
	page.leaf = true

end

