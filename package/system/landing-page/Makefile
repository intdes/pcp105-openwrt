#
# Copyright (C) 2008-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

UCI_VERSION=2014-06-18
UCI_RELEASE=1

PKG_NAME:=landing-page
PKG_VERSION:=$(UCI_VERSION)$(if $(UCI_RELEASE),.$(UCI_RELEASE))
PKG_RELEASE:=2
PKG_REV:=e339407372ffc70b1451e4eda218c01aa95a6a7e

PKG_LICENSE:=LGPLv2.1 GPLv2
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Will ONeill <will@intellidesign.com.au>

PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/package.mk

# set to 1 to enable debugging
DEBUG=

define Package/landing-page
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=
  TITLE:=Web landing page
endef


define Build/Prepare
endef

define Build/Compile/Default

endef
Build/Compile = $(Build/Compile/Default)

define Package/landing-page/install
    $(CP) ./files/* $(1)/
endef

define Package/$(PKG_NAME)/postinst
    if [ -f /etc/config/uhttpd ]; then
		uci set uhttpd.main.index_page=landing.html
		uci commit
		/etc/init.d/uhttpd restart
    fi
	exit 0
endef


$(eval $(call BuildPackage,landing-page))
