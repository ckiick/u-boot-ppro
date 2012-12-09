
# u-boot configuration for the oxnas 820 (pogoplug) platform
# This won't work with medionnas or other oxnas devices, because of board rev.

# CONFIG_SYS_TEXT_BASE = 0x60d00000

# mem size in megabytes
MEM_SIZE ?= 128

SET_LINUX_MEM_SIZE ?= 1

# CONFIG_USB_EHCI = y
# CONFIG_USB_EHCI_OXNAS = y
# CONFIG_USB_STORAGE = y
# CONFIG_EHCI_IS_TDI = y

