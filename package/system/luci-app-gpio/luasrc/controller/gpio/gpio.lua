-- Copyright 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.gpio.gpio", package.seeall)

function index()
	local page

	page = entry({"admin", "system", "gpio"}, cbi("gpio/gpio", {hideapplybtn=true, hidesavebtn=true, hideresetbtn=true}), _("GPIO Configuration"))
	page.leaf = true

end

