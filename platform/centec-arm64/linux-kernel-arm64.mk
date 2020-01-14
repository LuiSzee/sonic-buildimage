# linux kernel package for centec arm64

export KVERSION= 4.9.62
export LINUX_KERNEL_VERSION=4.9.62_4.9.62

LINUX_KERNEL = linux-image-4.9.62-arm64.deb
$(LINUX_KERNEL)_URL = https://github.com/CentecNetworks/sonic-binaries/raw/master/$(PLATFORM_ARCH)/kernel/$(LINUX_KERNEL)
SONIC_ONLINE_DEBS += $(LINUX_KERNEL)
SONIC_STRETCH_DEBS += $(LINUX_KERNEL)

LINUX_KERNEL_DEBIMG = linux-image-4.9.62_4.9.62-1_arm64.deb
$(LINUX_KERNEL_DEBIMG)_URL = https://github.com/CentecNetworks/sonic-binaries/raw/master/$(PLATFORM_ARCH)/kernel/$(LINUX_KERNEL_DEBIMG)
SONIC_ONLINE_DEBS += $(LINUX_KERNEL_DEBIMG)
SONIC_STRETCH_DEBS += $(LINUX_KERNEL_DEBIMG)

LINUX_KERNEL_HEADERS = linux-headers-4.9.62_4.9.62-1_arm64.deb
$(LINUX_KERNEL_HEADERS)_URL = https://github.com/CentecNetworks/sonic-binaries/raw/master/$(PLATFORM_ARCH)/kernel/$(LINUX_KERNEL_HEADERS)
SONIC_ONLINE_DEBS += $(LINUX_KERNEL_HEADERS)
SONIC_STRETCH_DEBS += $(LINUX_KERNEL_HEADERS)