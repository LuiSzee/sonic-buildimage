# Centec E550-24T16Y Platform modules


CENTEC_E550_24T16Y_PLATFORM_MODULE_VERSION =1.1
CENTEC_E550_24X8Y2C_PLATFORM_MODULE_VERSION =1.1

export CENTEC_E550_24T16Y_PLATFORM_MODULE_VERSION
export CENTEC_E550_24X8Y2C_PLATFORM_MODULE_VERSION

CENTEC_E550_24T16Y_PLATFORM_MODULE = platform-modules-e550-24t16y_$(CENTEC_E550_24T16Y_PLATFORM_MODULE_VERSION)_arm64.deb

$(CENTEC_E550_24T16Y_PLATFORM_MODULE)_SRC_PATH = $(PLATFORM_PATH)/sonic-platform-modules-e550
$(CENTEC_E550_24T16Y_PLATFORM_MODULE)_PLATFORM = arm64-centec_e550_24t16y-r0
$(CENTEC_E550_24T16Y_PLATFORM_MODULE)_DEPENDS += $(LINUX_HEADERS) $(LINUX_HEADERS_COMMON) $(LINUX_KERNEL_HEADERS)
SONIC_STRETCH_DEBS += $(CENTEC_E550_24T16Y_PLATFORM_MODULE)
SONIC_DPKG_DEBS += $(CENTEC_E550_24T16Y_PLATFORM_MODULE)

CENTEC_E550_24X8Y2C_PLATFORM_MODULE = platform-modules-e550-24x8y2c_$(CENTEC_E550_24X8Y2C_PLATFORM_MODULE_VERSION)_arm64.deb
$(CENTEC_E550_24X8Y2C_PLATFORM_MODULE)_PLATFORM = arm64-centec_e550_24x8y2c-r0
$(eval $(call add_extra_package,$(CENTEC_E550_24T16Y_PLATFORM_MODULE),$(CENTEC_E550_24X8Y2C_PLATFORM_MODULE)))
