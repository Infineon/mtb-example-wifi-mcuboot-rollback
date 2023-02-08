################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Top-level application make file.
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

# Include the application make files
include ../common.mk

################################################################################
# Basic Configuration
################################################################################

MTB_TYPE=PROJECT

# Core processor
CORE=CM4
CORE_NAME=CM4_0

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=factory_app_cm4

# Starting address of the CM4 app or the offset at which the header of an image
# will begin. Image = Header + App + TLV + Trailer. See MCUboot documenation for
# details.
# New relocated address = ORIGIN + HEADER_OFFSET
# ORIGIN is defined in the CM4 linker script and is usually the address next to
# the end of CM0+ image.
# Refer MCUBoot 'ExternalMemory.md' for details on the calculations
HEADER_OFFSET ?= 0x7FE8000

# Erase value, used in building the final image for the specified target.
# use 0 for internal flash and 0xff for external flash.
# factory_app is built for exetrnal flash by default.
ERASED_VALUE = 0xff

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

# Set this to 1, if you would like to print debug prints
DEBUG_PRINT=0

# define the location of bootloder where all necessary configs are defined.
BOOTLOADER_PATH=../bootloader_cm0p

# Include the flash map configuration and make configs
include $(BOOTLOADER_PATH)/shared_config.mk

# factory_app size should be defined based on the external memory erase sector size
# considering 0x40000 sector size, allocating 4 sectors (1MB) for the factory-image.
ifeq ($(USE_EXTERNAL_FLASH), 1)
FACT_APP_SIZE=0x200000
else
FACT_APP_SIZE=0x100000
endif

################################################################################
# Advanced Configuration
################################################################################

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS+=FREERTOS PSOC6HAL LWIP MBEDTLS SECURE_SOCKETS OTA_MQTT

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS+=

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES+=./configs

# Custom configuration of mbedtls library.
MBEDTLSFLAGS = MBEDTLS_USER_CONFIG_FILE='"./configs/mbedtls_user_config.h"'

# Add additional defines to the build process (without a leading -D).
DEFINES=$(MBEDTLSFLAGS) CYBSP_WIFI_CAPABLE CY_RETARGET_IO_CONVERT_LF_TO_CRLF
DEFINES+=CY_RTOS_AWARE HTTP_DO_NOT_USE_CUSTOM_CONFIG 

# CY8CPROTO-062-4343W board shares the same GPIO for the user button (SW2)
# and the CYW4343W host wake up pin. Since this example uses the GPIO for
# interfacing the user button, the SDIO interrupt to wake up the host is
# disabled by setting CY_WIFI_HOST_WAKE_SW_FORCE to '0'
ifeq ($(TARGET), CY8CPROTO-062-4343W)
DEFINES+=CY_WIFI_HOST_WAKE_SW_FORCE=0
endif

# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT=hardfp

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS=

# Additional / custom libraries to link in to the application.
LDLIBS=

# Path to the linker script to use (if empty, use the default linker script).
LINKER_SCRIPT=

# Custom pre-build commands to run.
PREBUILD=

# Custom post-build commands to run.
POSTBUILD=

###############################################################################
#
# OTA Setup
#
###############################################################################

# Set to 1 to add OTA defines, sources, and libraries (must be used with MCUBoot)
# NOTE: Extra code must be called from your app to initialize the OTA middleware.
OTA_SUPPORT=1

# Component for adding platform-specific code
# ex: source/port_support/mcuboot/COMPONENT_OTA_PSOC_062/flash_qspi/flash_qspi.c
COMPONENTS+=OTA_PSOC_062

# Set Platform type (added to defines and used when finding the linker script)
# Ex: PSOC_062_2M, PSOC_062_1M, PSOC_062_512K
# Only one of the following two if conditions will be true
OTA_PLATFORM=$(if $(filter PSOC6_02,$(DEVICE_COMPONENTS)),PSOC_062_2M,$(if $(filter PSOC6_03,$(DEVICE_COMPONENTS)),PSOC_062_512K))

# used the same json file for all application
OTA_FLASH_MAP=$(BOOTLOADER_PATH)/flashmap/$(FLASH_MAP)

# Version of the app
APP_VERSION_MAJOR?=1
APP_VERSION_MINOR?=0
APP_VERSION_BUILD?=0

# Path of the key file passed as a parameter to the Python module "imgtool" for
# signing the image. Name of the key is file is set in bootloader_cm0p/shared_config.mk
# as a common parameter to all three applications
KEY_FILE_PATH=$(BOOTLOADER_PATH)/keys

###############################################################################
#
# OTA Functionality support
#
###############################################################################
ifeq ($(OTA_SUPPORT),1)

    # Build location local to this root directory.
    CY_BUILD_LOC:=./build
    
    # Add OTA_PLATFORM in DEFINES for platform-specific code
    # ex: source/port_support/mcuboot/COMPONENT_OTA_PSOC_062/flash_qspi/flash_qspi.c
    DEFINES+=$(OTA_PLATFORM)

    # for use when running flashmap.py
    FLASHMAP_PLATFORM=$(OTA_PLATFORM)

    FLASHMAP_PYTHON_SCRIPT=flashmap.py
    flash_map_mk_exists=$(shell if [ -s "flashmap.mk" ]; then echo "success"; fi )
    ifneq ($(flash_map_mk_exists),)
        $(info include flashmap.mk)
        include ./flashmap.mk
    endif # flash_map_mk_exists
endif # OTA_SUPPORT

# Defines for anycloud-ota with MCUBOOT
#
# Additional / custom linker flags.
# This needs to be before finding LINKER_SCRIPT_WILDCARD as we need the extension defined
ifeq ($(TOOLCHAIN),GCC_ARM)

CY_ELF_TO_HEX_TOOL=$(CY_CROSSPATH)/bin/arm-none-eabi-objcopy
CY_ELF_TO_HEX_OPTIONS=-O ihex
LDFLAGS+=-Wl,--defsym=CM0P_FLASH_SIZE=$(FLASH_AREA_BOOTLOADER_SIZE)
LDFLAGS+=-Wl,--defsym=CM0P_RAM_SIZE=$(BOOTLOADER_APP_RAM_SIZE)
LDFLAGS+=-Wl,--defsym=CM4_FLASH_SIZE=$(FLASH_AREA_IMG_1_PRIMARY_SIZE)
LDFLAGS+=-Wl,--defsym=CY_BOOT_PRIMARY_1_SIZE=$(FLASH_AREA_IMG_1_PRIMARY_SIZE)
LDFLAGS+=-Wl,--defsym=MCUBOOT_HEADER_SIZE=$(MCUBOOT_HEADER_SIZE)
LDFLAGS+=-Wl,--defsym=CY_FACT_APP_SIZE=$(FACT_APP_SIZE)
LDFLAGS+=-Wl,--defsym=CY_EXT_FLASH_SIZE=$(EXTERNAL_FLASH_SIZE)

#Linker flags
LDFLAGS+=-Wl,--undefined=uxTopUsedPriority

else
$(error Only GCC_ARM is supported at this point)
endif #GCC_ARM

#Additional defines for the MCUBoot
DEFINES+=FLASH_AREA_BOOTLOADER_DEV_ID="$(FLASH_AREA_BOOTLOADER_DEV_ID)"\
         FLASH_AREA_BOOTLOADER_START=$(FLASH_AREA_BOOTLOADER_START)\
         FLASH_AREA_BOOTLOADER_SIZE=$(FLASH_AREA_BOOTLOADER_SIZE)\
         FLASH_AREA_IMG_1_PRIMARY_DEV_ID="$(FLASH_AREA_IMG_1_PRIMARY_DEV_ID)"\
         FLASH_AREA_IMG_1_PRIMARY_START=$(FLASH_AREA_IMG_1_PRIMARY_START) \
         FLASH_AREA_IMG_1_PRIMARY_SIZE=$(FLASH_AREA_IMG_1_PRIMARY_SIZE) \
         FLASH_AREA_IMG_1_SECONDARY_DEV_ID="$(FLASH_AREA_IMG_1_SECONDARY_DEV_ID)"\
         FLASH_AREA_IMG_1_SECONDARY_SIZE=$(FLASH_AREA_IMG_1_SECONDARY_SIZE) \
         FLASH_AREA_IMAGE_SCRATCH_SIZE=$(FLASH_AREA_IMAGE_SCRATCH_SIZE)

# Defines to place Secondary slot in External Flash just 
# after factory app image.
ifeq ($(USE_EXTERNAL_FLASH),1)
DEFINES+=OTA_USE_EXTERNAL_FLASH=1 \
         FLASH_AREA_IMG_1_SECONDARY_START=$(FLASH_AREA_IMG_1_SECONDARY_START)
else
DEFINES+=FLASH_AREA_IMG_1_SECONDARY_START=$(FLASH_AREA_IMG_1_SECONDARY_START)
endif

ifeq ($(DEBUG_PRINT),1)
DEFINES+=DEBUG_PRINT=1
endif

# Application version details
DEFINES+=APP_VERSION_MAJOR=$(APP_VERSION_MAJOR)\
         APP_VERSION_MINOR=$(APP_VERSION_MINOR)\
         APP_VERSION_BUILD=$(APP_VERSION_BUILD)

# signed image creation
BINARY_OUT_PATH=./build/$(TARGET)/$(CONFIG)/$(APPNAME)
IMGTOOL_PATH=$(SEARCH_mcuboot)/scripts/imgtool.py
CY_BUILD_VERSION=$(APP_VERSION_MAJOR).$(APP_VERSION_MINOR).$(APP_VERSION_BUILD)
SIGN_ARGS=sign --header-size $(MCUBOOT_HEADER_SIZE) --pad-header --align 8 -v $(CY_BUILD_VERSION) \
               -S $(FLASH_AREA_IMG_1_PRIMARY_SIZE) -M $(MCUBOOT_MAX_IMG_SECTORS) --overwrite-only -R $(ERASED_VALUE) -k $(KEY_FILE_PATH)/$(SIGN_KEY_FILE).pem

# Python path definition
ifeq ($(OS),Windows_NT)
PYTHON_PATH?=python
else
PYTHON_PATH?=python3
endif

POSTBUILD=\
cp -f $(BINARY_OUT_PATH).hex $(BINARY_OUT_PATH)_raw.hex;\
$(CY_ELF_TO_HEX_TOOL) --change-addresses=$(HEADER_OFFSET) $(CY_ELF_TO_HEX_OPTIONS) $(BINARY_OUT_PATH).elf $(BINARY_OUT_PATH)_unsigned.hex;\
$(PYTHON_PATH) $(IMGTOOL_PATH) $(SIGN_ARGS) $(BINARY_OUT_PATH)_unsigned.hex $(BINARY_OUT_PATH).hex;

################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION> field
# begins with $$ASSET_REPO$$, then the repo is deposited in the path specified by
# the CY_GETLIBS_SHARED_PATH variable. The default location is one directory level
# above the current app directory.
# This is used with CY_GETLIBS_SHARED_NAME variable, which specifies the directory name.
CY_GETLIBS_SHARED_PATH=../..

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

# Absolute path to the compiler's "bin" directory.
#
# The default depends on the selected TOOLCHAIN (GCC_ARM uses the ModusToolbox
# IDE provided compiler by default).
CY_COMPILER_PATH=

include $(CY_TOOLS_DIR)/make/start.mk

###############################################################################
#
# OTA flashmap parser must be run after start.mk so that libs/mtb.mk is valid
#
###############################################################################

ifeq ($(OTA_SUPPORT),1)
#
# Only when we are in the correct build pass
#
    ifneq ($(MAKECMDGOALS),getlibs)
    ifneq ($(MAKECMDGOALS),get_app_info)
    ifneq ($(MAKECMDGOALS),printlibs)
    ifneq ($(FLASHMAP_PYTHON_SCRIPT),)
    ifneq ($(OTA_FLASH_MAP),)
    ifeq ($(CY_PYTHON_PATH),)
        CY_PYTHON_PATH=$(shell which python)
    endif
        $(info "flashmap.py $(CY_PYTHON_PATH) $(BOOTLOADER_PATH)/scripts/$(FLASHMAP_PYTHON_SCRIPT) -p $(FLASHMAP_PLATFORM) -i $(OTA_FLASH_MAP) > flashmap.mk")
        $(shell $(CY_PYTHON_PATH) $(BOOTLOADER_PATH)/scripts/$(FLASHMAP_PYTHON_SCRIPT) -p $(FLASHMAP_PLATFORM) -i $(OTA_FLASH_MAP) > flashmap.mk)
        flash_map_status=$(shell if [ -s "flashmap.mk" ]; then echo "success"; fi )
        ifeq ($(flash_map_status),)
            $(info "")
            $(error Failed to create flashmap.mk !)
            $(info "")
        else
            $(info include flashmap.mk)
            include ./flashmap.mk
        endif # flash_map_status
    endif # OTA_FLASH_MAP
    endif # FLASHMAP_PYTHON_SCRIPT
    endif # NOT getlibs
    endif # NOT get_app_info
    endif # NOT printlibs
    
endif # OTA_SUPPORT