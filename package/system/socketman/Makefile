# Copyright (C) 2016 Cucumber WiFi
#
# This is free software, licensed under the GNU General Public License v2.

CT_ARCH += "-D__OPENWRT__"

include $(TOPDIR)/rules.mk

PKG_NAME:=SocketMan
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=c49afc60dd9d3c7d795226888b1a66cf61bb1cef
PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE_URL:=https://github.com/cucumber-tony/SocketMan.git
PKG_SOURCE:=$(PKG_NAME).tar.gz
PKG_VERSION:=v1

PATCH_DIR:=patches

include $(INCLUDE_DIR)/package.mk

define Package/socketman
SECTION:=cucumber
CATEGORY:=Cucumber Tony
DEPENDS:=+zlib +libjson-c +curl +libcurl +libopenssl +libpthread +libmosquitto +@LIBCURL_OPENSSL +libc +libnl
TITLE:=SocketMan communication package for Cucumber Tony
URL:=http://www.cucumberwifi.io
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include/libnl3 \
	-I$(STAGING_DIR)/usr/include


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/build
	mkdir -p $(PKG_BUILD_DIR)/obj
	tar -C $(DL_DIR) -xzvf $(DL_DIR)/$(PKG_NAME).tar.gz
	$(CP) $(DL_DIR)/$(PKG_NAME)/src/* $(PKG_BUILD_DIR)/
	$(CP) patches/001-socketman.patch $(PKG_BUILD_DIR)/
	cd $(PKG_BUILD_DIR) && patch -p1 <001-socketman.patch
endef

# define Package/socketman/conffiles
# /etc/config/socketman
# endef

define Package/socketman/description
 This package contains the communication package of CT
endef

define Package/socketman/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/socketman $(1)/usr/bin/socketman
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/socketman.init $(1)/etc/init.d/socketman
endef

$(eval $(call BuildPackage,socketman))

#.PHONY: clean
#clean:
#	rm -rf $(DL_DIR)/$(PKG_NAME)*
