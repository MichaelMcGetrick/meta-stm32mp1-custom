SUMMARY = "My Custom Linux Image."

IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"

IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit core-image
inherit extrausers


# Set rootfs to 200 MiB by default
IMAGE_OVERHEAD_FACTOR ?= "1.0"
IMAGE_ROOTFS_SIZE ?= "204800"

# Change root password (note the capital -P)
#EXTRA_USERS_PARAMS = "\
#  usermod -P 'toor'  root \
#  "

#Include custom application:
IMAGE_INSTALL += "gettime spiapp"


#Add kernel moodules:
IMAGE_INSTALL:append = " kernel-modules"

#Add required modules for Wifi
DISTRO_FEATURES:append = " wifi"
IMAGE_INSTALL:append = " linux-firmware-bluetooth-bcm4343"
IMAGE_INSTALL:append = " linux-firmware-bcm43430"
#IMAGE_INSTALL:append = " dhcp-client"
IMAGE_INSTALL:append = " iw"
IMAGE_INSTALL:append = " wpa-supplicant"
IMAGE_INSTALL:append = " wireless-regdb-static"

#IMAGE_INSTALL += "kernel-module-r8188eu \
#                  linux-firmware-rtl8188 \
#                  dhcp-client \
#                  iw \
#                  wpa-supplicant \
#                  wireless-regdb-static"

# Autoload WiFi driver on boot
KERNEL_MODULE_AUTOLOAD +=  "brcmfmac"

