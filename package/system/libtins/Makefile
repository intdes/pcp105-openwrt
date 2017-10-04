#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libtins
PKG_VERSION:=3.5
PKG_RELEASE:=3.5

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/mfontanini/libtins.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=37c92fcf5c83b034a280fd6070fb467e3d0c87bd
PKG_MAINTAINER:=Matias Fontanini
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libtins
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libopenssl +libpcap +libstdcpp
  TITLE:=High-level, multiplatform C++ network packet sniffing and crafting library.
endef

CMAKE_OPTIONS += -D_RUN_RESULT_VAR=0

define Package/libtins/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libtins))
