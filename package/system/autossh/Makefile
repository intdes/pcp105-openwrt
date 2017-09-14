#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autossh
PKG_RELEASE:=1
PKG_VERSION:=1.4e

PKG_MAINTAINER:=Carson Harding

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://www.harding.motd.ca/autossh

include $(INCLUDE_DIR)/package.mk

define Package/autossh
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=
  TITLE:=autossh utility
endef

define Package/autossh/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/autossh $(1)/usr/bin
endef

$(eval $(call BuildPackage,autossh))

