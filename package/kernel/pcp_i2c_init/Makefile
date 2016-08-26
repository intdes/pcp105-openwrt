#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pcp-i2c-init
PKG_VERSION:=0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/pcp-i2c-init
  SUBMENU:=Other modules
  TITLE:=I2C peripheral setup for PCP105
  FILES:=$(PKG_BUILD_DIR)/pcp_i2c_init.ko
  AUTOLOAD:=$(call AutoLoad,10,pcp_i2c_init,1)
endef

define KernelPackage/pcp-i2c-init/description
I2C peripheral setup for the PCP105.
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp src/Makefile src/pcp_i2c_init.c $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(KERNEL_MAKEOPTS) SUBDIRS="$(PKG_BUILD_DIR)" modules
endef

define KernelPackage/pcp-i2c-init/conffiles
/etc/config/pcp-i2c-init
endef

$(eval $(call KernelPackage,pcp-i2c-init))
