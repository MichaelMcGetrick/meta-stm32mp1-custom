#FILESEXTRAPATHS_prepend := "${THISDIR}:"
#Change due to new override symbols since Honister
FILESEXTRAPATHS:prepend := "${THISDIR}:"
SRC_URI += "file://0001-add-i2c5-userspace-dts.patch" 
#SRC_URI += "file://0002-add-spi4-userspace-dts.patch"
SRC_URI += "file://0003-add-spi4-custom_userspace-dts.patch"
SRC_URI += "file://0001-mcp-char-driver.patch"
# Apply default configuration
#KERNEL_DEFCONFIG_stm32mp1 = "defconfig"
