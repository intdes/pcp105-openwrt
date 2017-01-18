#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=hyrax-mcu
PKG_VERSION:=0.1
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk

define KernelPackage/hyrax-mcu
  SUBMENU:=Other modules
  TITLE:=MCU interface for the PCP105
  FILES:=$(PKG_BUILD_DIR)/hyrax_mcu.ko
  AUTOLOAD:=$(call AutoLoad,62,hyrax_mcu,1)
endef

define KernelPackage/hyrax-mcu/description
I2C peripheral setup for the PCP105.
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(KERNEL_MAKEOPTS) SUBDIRS="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,hyrax-mcu))
