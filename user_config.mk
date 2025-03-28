################################################################################
# \file user_config.mk
# \version 1.0
#
# \brief
# Holds configuration and error checking that are common to all three application.
# Ensure that this file is included in the application's
# Makefile after other application-specific variables such as TARGET are defined.
#
################################################################################
# \copyright
# Copyright 2021-2025, Cypress Semiconductor Corporation (an Infineon company)
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

# Include the common make file
include ../common.mk

################################################################################
# Shared Configuration
################################################################################

# Device family name
# E.g. FAMILY=PSOC6
FAMILY=PSOC6

# Device platform name
# E.g. PLATFORM=PSOC_062_2M, PLATFORM=PSOC_062_1M, PLATFORM=PSOC_062_512K
PLATFORM=PSOC_062_2M

# Flashmap JSON file name
FLASH_MAP=psoc62_swap_single_smif.json

# Name of the key file, used in two places.
# 1. #included in keys.c for embedding it in the Bootloader app, used for image
#    authentication.
# 2. Passed as a parameter to the parameter to the imgtool or cysecuretools for signing the image
#    in the user and factory app Makefile. The path of this key file is set in the user
#    app Makefile.
SIGN_KEY_FILE=cypress-test-ec-p256

# Path of the key file passed as a parameter to the imgtool or cysecuretools for
# signing the image. Name of the key file is set in user_config.mk
# as a common parameter to both the Bootloader app and Blinky app.
SIGN_KEY_FILE_PATH=../keys

# Flash and RAM size for MCUBoot Bootloader app run by CM0+
BOOTLOADER_APP_RAM_SIZE=0x20000

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

# Default upgrade method
PLATFORM_DEFAULT_USE_OVERWRITE=0

# Default memory single chunk size
PLATFORM_CHUNK_SIZE=0x200

# Minimum erase size of underlying memory hardware
PLATFORM_MEMORY_ALIGN=0x200

ifeq ($(FAMILY), PSOC6)
# Maximum trailer page size for PSOC 6 devices
PLATFORM_MAX_TRAILER_PAGE_SIZE=0x200
endif

# Encrypted image support
# This code example not supported the encrypted image at the moment
ENC_IMG=0

ifneq ($(ENC_IMG), 0)
$(error This code example not supported the encrypted image at the moment)
endif

################################################################################
# Bootloader App Configuration
################################################################################

# Downgrade prevention, to avoid older firmware version for upgrade
USE_SW_DOWNGRADE_PREV?=1

# When set to `1` and Swap mode is enabled, the application in the secondary slot will overwrite the primary slot if the primary slot application is invalid.
USE_BOOTSTRAP?=1

################################################################################
# Factory App Configuration
################################################################################

# image type can be BOOT or UPGRADE
IMG_TYPES:=BOOT UPGRADE

# Supported values: BOOT, UPGRADE
# The Factory app defines the LED period differently depending on whether the
# image is BOOT or UPGRADE.
# BOOT: Use when image is built for the primary slot.
#       --pad argument is not passed to the imgtool or cysecuretools.
# UPGRADE: Use when image is built for secondary slot.
#          --pad argument is passed to the imgtool or cysecuretools.
IMG_TYPE?=UPGRADE

ifneq ($(filter $(IMG_TYPE), $(IMG_TYPES)),)
else
$(error Set IMG_TYPE to be either BOOT or UPGRADE)
endif

# Image ID
# Below flag value should correspond to the `application_#` number of JSON flash map file used for the build
IMG_ID=1

ifneq ($(IMG_ID), 1)
$(error This code example not supported the multi-image configuration at the moment)
endif

# Define the size of the external flash based on target
# Print error for targets, which are not supported by this example.
ifeq ($(PLATFORM), PSOC_062_2M)
EXTERNAL_FLASH_SIZE=0x4000000
else
$(error Target PLATFORM $(PLATFORM) not supported)
endif

# factory_app size should be defined based on the external memory erase sector size
FACT_APP_SIZE=0x200000
