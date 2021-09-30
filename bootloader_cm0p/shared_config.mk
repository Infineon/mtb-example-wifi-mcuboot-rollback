################################################################################
# \file shared_config.mk
# \version 1.0
#
# \brief
# Holds configuration and error checking that are common to all three application.
# Ensure that this file is included in the application's
# Makefile after other application-specific variables such as TARGET, 
# USE_EXT_FLASH are defined. 
#
################################################################################
# \copyright
# Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
# SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

# Name of the key file, used in two places.
# 1. #included in keys.c for embedding it in the Bootloader app, used for image
#    authentication.
# 2. Passed as a parameter to the Python module "imgtool" for signing the image
#    in the application(user and factory) Makefile.
#    The path of this key file is set in respective application Makefile.
SIGN_KEY_FILE=cypress-test-ec-p256


# NOTE: Following variables are passed as options to the linker. 
# Ensure that the values have no trailing white space. Linker will throw error
# otherwise. 

# The bootloader app is run by CM0+. Therefore, the scratch size and the
# bootloader size are used with the linker script for the bootloader app. 
# The slot sizes (primary_1, secondary_1, primary_2, and secondary_2) are used
# with the linker script for the applications run by CM4.

# Flash size of MCUBoot Bootloader app run by CM0+
BOOTLOADER_APP_FLASH_SIZE=0x18000

# RAM size of MCUBoot Bootloader app run by CM0+
BOOTLOADER_APP_RAM_SIZE=0x20000

# Slot size includes the flash size of the Blinky App run by CM4
# One slot = MCUboot Header + App + TLV + Trailer (Trailer is not present for BOOT image)
# The RAM size of the Blinky App is set in the linker script as given below. 
# CM4 RAM size = Total RAM available in the device - 2 KB reserved for system use - BOOTLOADER_APP_RAM_SIZE.
# Slot size is kept higher only to utilize the larger space available with the external memories. 
ifeq ($(USE_EXT_FLASH), 1)
MCUBOOT_SLOT_SIZE=0x1E0000
else
MCUBOOT_SLOT_SIZE=0xEE000
endif

# Size of the scratch area used by MCUboot while swapping the image between
# primary slot and secondary slot. 
MCUBOOT_SCRATCH_SIZE=0x1000

# MCUBoot header size
# Must be a multiple of 1024 because of the following reason. 
# CM4 image starts right after the header and the CM4 image begins with the
# interrupt vector table. The starting address of the table must be 1024-bytes
# aligned. See README.md for details.
# Number of bytes to be aligned to = Number of interrupt vectors x 4 bytes.
# (1024 = 256 x 4)
# 
# Header size is used in two places. 
# 1. The location of CM4 image is offset by the header size from the ORIGIN
# value specified in the linker script. 
# 2. Passed to the imgtool while signing the image. The imgtool fills the space
# of this size with zeroes and then adds the actual header from the beginning of
# the image.
MCUBOOT_HEADER_SIZE=0x400 

# Maximum number of flash sectors (or rows) per image slot.
# The maximum number of flash sectors for which swap status is tracked in
# the image trailer. 
# In the bootloader app, this value is used in DEFINE+= to override the macro
# with the same name in "mcuboot/boot/cypress/MCUBootApp/config/mcuboot_config/mcuboot_config.h".
# In the blinky app, this value is passed with "-M" option to the imgtool while 
# signing the image. imgtool adds padding in the trailer area
# depending on this value. 
# This value can be simply set to MCUBOOT_SLOT_SIZE/FLASH_ROW_SIZE.
# For PSoC 6, FLASH_ROW_SIZE=512 bytes.
MCUBOOT_SECTOR_SIZE = 512

ifeq ($(SWAP_UPGRADE), 1)
ifeq ($(USE_EXT_FLASH), 1)
# SWAP with external memory will use 0x40000 (for S25FL512S) as sector size.
MCUBOOT_SECTOR_SIZE = 0x40000
endif
endif

# Minimum number of sectors accecpted by MCUBoot Library is 32
MIN_IMG_SECTORS = 32

# Number of flash sectors per image slot
FLASH_IMG_SECTORS = $(shell expr $$(( $(MCUBOOT_SLOT_SIZE) / $(MCUBOOT_SECTOR_SIZE) )) )

# Maximum number of flash sectors (or rows) per image slot.
MCUBOOT_MAX_IMG_SECTORS = $(shell expr $$(( $(MIN_IMG_SECTORS) > $(FLASH_IMG_SECTORS) ? $(MIN_IMG_SECTORS) : $(FLASH_IMG_SECTORS) )) )

# Define Internal Flash Size
INT_FLASH_SIZE=0x200000

# factory_app size should be defined based on the external memory erase sector size
# considering 0x40000 sector size, allocating 4 sectors (1MB) for the factory-image.
ifeq ($(USE_EXT_FLASH), 1)
FACT_APP_SIZE=0x200000
else
FACT_APP_SIZE=0x100000
endif

# Define the size of the external flash based on target
# Print error for Targets, which are not supported by this example.
ifeq ($(TARGET), $(filter $(TARGET),CY8CPROTO-062-4343W CY8CKIT-062S2-43012))
EXTERNAL_FLASH_SIZE=0x4000000
else
$(error TARGET $(TARGET) not supported)
endif
