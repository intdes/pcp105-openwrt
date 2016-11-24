#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=hyrax-init
PKG_VERSION:=0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/hyrax-init
  SUBMENU:=Other modules
  TITLE:=Peripheral setup for PCP105 and fit machine
  FILES:=$(PKG_BUILD_DIR)/hyrax_init.ko
#  AUTOLOAD:=$(call AutoLoad,10,hyrax_init,1)
endef

define KernelPackage/hyrax-init/description
I2C peripheral setup for the PCP105.
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp src/Makefile src/*.c $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(KERNEL_MAKEOPTS) SUBDIRS="$(PKG_BUILD_DIR)" modules
endef

define KernelPackage/hyrax-init/conffiles
/etc/config/hyrax-init
endef

$(eval $(call KernelPackage,hyrax-init))
