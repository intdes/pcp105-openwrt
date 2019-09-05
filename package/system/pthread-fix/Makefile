
include $(TOPDIR)/rules.mk

PKG_NAME:=pthread-fix
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Adam White <adam.white@intellidesign.com.au>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/pthread-fix
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libc
  TITLE:=libpthread fix
endef

define Package/pthread-fix/description
	Fixes compiling with -lpthread as libpthread is provided by libc
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/pthread-fix/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(AR) -rc $(1)/usr/lib/libpthread.a
endef

$(eval $(call BuildPackage,pthread-fix))
