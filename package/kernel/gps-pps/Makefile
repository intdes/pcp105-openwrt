#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=gps-pps
PKG_VERSION:=0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/gps-pps
  SUBMENU:=Other modules
  TITLE:=GPS PPS support for PCP105
  FILES:=$(PKG_BUILD_DIR)/gps_pps_data.ko
  AUTOLOAD:=$(call AutoLoad,50,gps_pps_data,1)
endef

define KernelPackage/gps-pps/description
GPS PPS support for the PCP105.
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp src/Makefile src/gps_pps_data.c $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(KERNEL_MAKEOPTS) SUBDIRS="$(PKG_BUILD_DIR)" modules
endef

define KernelPackage/gps-pps/conffiles
/etc/config/gps-pps
endef

#define KernelPackage/gps-pps/install
#	$(INSTALL_DIR) $(1)/etc/hotplug.d/net $(1)/etc/init.d $(1)/etc/config
#	$(INSTALL_DATA) ./files/gps-pps.hotplug $(1)/etc/hotplug.d/net/50-gps-pps
#	$(INSTALL_BIN) ./files/gps-pps.init $(1)/etc/init.d/gps-pps
#	$(INSTALL_DATA) ./files/gps-pps.config $(1)/etc/config/gps-pps
#endef

$(eval $(call KernelPackage,gps-pps))
