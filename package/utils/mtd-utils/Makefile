#
# Copyright (C) 2009-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtd-utils
PKG_REV:=7d2839b8dede3ae368780364b4a07473d2303219
PKG_VERSION:=1.5.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=git://git.infradead.org/mtd-utils.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_MIRROR_MD5SUM:=69b68adcfcbe786584771e5978534ab4

PKG_INSTALL:=1

PKG_BUILD_DEPENDS:=util-linux liblzo zlib

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=John Crispin <blogic@openwrt.org>

include $(INCLUDE_DIR)/package.mk

define Package/mtd-utils
  TITLE:=Utilities for ubi info/debug
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=http://www.linux-mtd.infradead.org/
endef

define Package/mtd-utils/description
  Utilities for manipulating memory technology devices.
endef

MAKE_FLAGS += \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	BUILDDIR="$(PKG_BUILD_DIR)" \
	LDLIBS+="$(LIBGCC_S)" \
	WITHOUT_XATTR=1 \
	WITHOUT_LZO=1

define Package/mtd-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/{flash_erase,flash_eraseall,nandwrite} $(1)/usr/sbin/
endef


$(eval $(call BuildPackage,mtd-utils))
