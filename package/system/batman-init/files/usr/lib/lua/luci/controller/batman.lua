--- IntelliDesign Pty Ltd
--- Copyright 2017 - All rights reserved
--- Custom batman-adv luci module
--- version 1.0

module("luci.controller.batman", package.seeall)

function index()
	entry( {"admin", "services", "batman"}, cbi("batman"), _("Batman") )
end
