################################################################################
# \file shared_config.mk
# \version 1.0
#
# \brief
# Holds configuration and error checking that are common to all three application.
# Ensure that this file is included in the application's
# Makefile after other application-specific variables such as TARGET are defined. 
#
################################################################################
# \copyright
# Copyright 2021-2023, Cypress Semiconductor Corporation (an Infineon company)
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

# Flashmap JSON file name
FLASH_MAP=psoc62_swap_single_smif.json

# Device family name. Ex: PSOC6, CYW20289
FAMILY=PSOC6

# Device platform name (Empty for CYW20289 device). 
# Ex: PLATFORM=PSOC_062_2M, PLATFORM=PSOC_062_1M, PLATFORM=PSOC_062_512K
PLATFORM=$(if $(filter PSOC6_02,$(DEVICE_COMPONENTS)),PSOC_062_2M,$(if $(filter PSOC6_03,$(DEVICE_COMPONENTS)),PSOC_062_512K,$(if $(filter PSOC6_01,$(DEVICE_COMPONENTS)),PSOC_062_1M)))

# Name of the key file, used in two places. 
# 1. #included in keys.c for embedding it in the Bootloader app, used for image
#    authentication. 
# 2. Passed as a parameter to the Python module "imgtool" for signing the image
#    in the application(user and factory) Makefile.
#    The path of this key file is set in respective application Makefile.
SIGN_KEY_FILE=cypress-test-ec-p256

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

# Define the size of the external flash based on target
# Print error for Targets, which are not supported by this example.
ifeq ($(TARGET), $(filter $(TARGET),  CY8CPROTO-062-4343W-NEW APP_CY8CPROTO-062-4343W-NEW CY8CPROTO-062-4343W CY8CKIT-062S2-43012 APP_CY8CPROTO-062-4343W APP_CY8CKIT-062S2-43012 CY8CEVAL-062S2-LAI-4373M2 APP_CY8CEVAL-062S2-LAI-4373M2))
EXTERNAL_FLASH_SIZE=0x4000000
else
$(error TARGET $(TARGET) not supported)
endif