#
# Copyright (C) 2008-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

UCI_VERSION=2014-06-18
UCI_RELEASE=1

PKG_NAME:=setmac
PKG_VERSION:=$(UCI_VERSION)$(if $(UCI_RELEASE),.$(UCI_RELEASE))
PKG_RELEASE:=5
PKG_REV:=e339407372ffc70b1451e4eda218c01aa95a6a7f

PKG_LICENSE:=LGPLv2.1 GPLv2
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Will ONeill <will@intellidesign.com.au>

PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/package.mk

# set to 1 to enable debugging
DEBUG=

define Package/setmac
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=
  TITLE:=Script for setting the MAC address on startup based on EEPROM settings
endef

define Build/Prepare
endef

define Build/Compile/Default

endef
Build/Compile = $(Build/Compile/Default)


define Package/setmac/install
	$(CP) ./files/* $(1)/
endef

define Package/setmac/postinst
#!/bin/sh	
if [ -f /etc/init.d/setmac.sh ]; then
	/etc/init.d/setmac.sh enable
fi
exit 0
endef

$(eval $(call BuildPackage,setmac))
