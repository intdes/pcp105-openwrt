#
# Copyright (C) 2008-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

UCI_VERSION=2014-06-18
UCI_RELEASE=1

PKG_NAME:=net-snmp-custom
PKG_VERSION:=$(UCI_VERSION)$(if $(UCI_RELEASE),.$(UCI_RELEASE))
PKG_RELEASE:=1
PKG_REV:=e339407372ffc70b1451e4eda218c01aa95a6a7e

PKG_LICENSE:=LGPLv2.1 GPLv2
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Will ONeill <will@intellidesign.com.au>

PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/package.mk

# set to 1 to enable debugging
DEBUG=

define Package/net-snmp-custom
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libnl-core +libsensors
  TITLE:=A custom version of net-snmp 5.7.3
endef


define Build/Prepare
endef

define Build/Compile/Default

endef
Build/Compile = $(Build/Compile/Default)

define Package/net-snmp-custom/install
    $(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,net-snmp-custom))
