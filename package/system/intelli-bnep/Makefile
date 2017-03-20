#
# Copyright (C) 2008-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

UCI_VERSION=2014-06-18
UCI_RELEASE=1

PKG_NAME:=intelli-bnep
PKG_VERSION:=$(UCI_VERSION)$(if $(UCI_RELEASE),.$(UCI_RELEASE))
PKG_RELEASE:=1
PKG_REV:=e339407372ffc70b1451e4eda218c01aa95a6a7e

PKG_LICENSE:=LGPLv2.1 GPLv2
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Joshua Donaldson <joshua@intellidesign.com.au>

PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/package.mk

# set to 1 to enable debugging
DEBUG=

define Package/intelli-bnep
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=
  TITLE:=intellidesign bnep package for bluetooth AP capabilities
endef


define Build/Prepare
endef

define Build/Compile/Default

endef
Build/Compile = $(Build/Compile/Default)

define Package/intelli-bnep/install
    $(CP) ./files/* $(1)/
endef

define Package/intelli-bnep/postinst
	#!/bin/sh
	uci set firewall.@defaults[0].forward='ACCEPT'
	uci set firewall.@zone[0].masq='1'
	uci set firewall.@zone[0].mtu_fix='1'
	uci add firewall zone
	uci set firewall.@zone[3]=zone
	uci set firewall.@zone[3].name='bnep'
	uci set firewall.@zone[3].input='ACCEPT'
	uci set firewall.@zone[3].output='ACCEPT'
	uci set firewall.@zone[3].network='bnep'
	uci set firewall.@zone[3].forward='ACCEPT'
	uci set firewall.@zone[3].masq='1'
	uci set firewall.@zone[3].mtu_fix='1'
	uci add firewall forwarding
	uci set firewall.@forwarding[4]=forwarding
	uci set firewall.@forwarding[4].dest='lan'
	uci set firewall.@forwarding[4].src='bnep'
	uci add firewall forwarding
	uci set firewall.@forwarding[5]=forwarding
	uci set firewall.@forwarding[5].dest='wan'
	uci set firewall.@forwarding[5].src='bnep'
	uci add firewall forwarding
	uci set firewall.@forwarding[6]=forwarding
	uci set firewall.@forwarding[6].dest='wifi'
	uci set firewall.@forwarding[6].src='bnep'
	uci add firewall forwarding
	uci set firewall.@forwarding[7]=forwarding
	uci set firewall.@forwarding[7].dest='bnep'
	uci set firewall.@forwarding[7].src='lan'
	uci add firewall forwarding
	uci set firewall.@forwarding[8]=forwarding
	uci set firewall.@forwarding[8].dest='bnep'
	uci set firewall.@forwarding[8].src='wan'
	uci add firewall forwarding
	uci set firewall.@forwarding[9]=forwarding
	uci set firewall.@forwarding[9].dest='bnep'
	uci set firewall.@forwarding[9].src='wifi'
	uci commit firewall

	uci delete dhcp.@dnsmasq[0].domainneeded='1'
	uci delete dhcp.@dnsmasq[0].boguspriv='1'
	uci delete dhcp.@dnsmasq[0].filterwin2k='0'
	uci delete dhcp.@dnsmasq[0].localise_queries='1'
	uci delete dhcp.@dnsmasq[0].expandhosts='1'
	uci delete dhcp.@dnsmasq[0].nonegcache='0'
	uci delete dhcp.@dnsmasq[0].resolvfile
	uci delete dhcp.@dnsmasq[0].readethers
	uci set dhcp.@dnsmasq[0].nonwildcard='0'
	uci add_list dhcp.@dnsmasq[0].server='8.8.8.8'
	uci add_list dhcp.@dnsmasq[0].server='8.8.4.4'
	uci set dhcp.@dnsmasq[0].localservice='0'
	uci set dhcp.@dnsmasq[0].noresolv='1'
	uci set dhcp.@dnsmasq[0].nohosts='1'
	uci commit dhcp

	/etc/init.d/dnsmasq restart
	/etc/init.d/firewall restart

	sleep 1

	new_name="Hyrax PCP-105 $$(( $$(i2cget  -y -f 0 0x50 0x07 b) + $$(i2cget -y -f 0 0x50 0x06 b) + $$(i2cget -y -f 0 0x50 0x05 b) ))"
	sed -i "/#Name =/a Name=$$new_name" /etc/bluetooth/main.conf
	sync

	hciconfig hci0 up
	hciconfig hci0 name "$$new_name"
	hciconfig hci0 down

	if [ -f /etc/init.d/bnep.sh ] ; then
		/etc/init.d/bnep.sh enable
		sleep 1
	fi
	exit 0
endef


$(eval $(call BuildPackage,intelli-bnep))
