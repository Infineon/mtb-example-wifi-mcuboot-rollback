################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Top-level application make file.
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

################################################################################
# Basic Configuration
################################################################################

# Target board/hardware (BSP)
#
# To change the target, it is recommended to use the Library manager
# ('make modlibs' from command line), which will also update Eclipse IDE launch
# configurations. If TARGET is manually edited, ensure TARGET_<BSP>.mtb with a
# valid URL exists in the application, run 'make getlibs' to fetch BSP contents
# and update or regenerate launch configurations for your IDE.
TARGET=CY8CPROTO-062-4343W

# Core processor
CORE?=CM4

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=factory_app_cm4

# Name of toolchain to use. Options include:
#
# GCC_ARM -- GCC provided with ModusToolbox IDE
# ARM     -- ARM Compiler (must be installed separately)
# IAR     -- IAR Compiler (must be installed separately)
#
# See also: CY_COMPILER_PATH below
TOOLCHAIN=GCC_ARM

# Default build configuration. Options include:
#
# Debug -- build with minimal optimizations, focus on debugging.
# Release -- build with full optimizations
# Custom -- build with custom configuration, set the optimization flag in CFLAGS
CONFIG?=Debug

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

# Set USE_EXT_FLASH to 1, if you would like to place new OTA image(secondary slot image)
# to external Flash.
USE_EXT_FLASH?=0

# Set this to 1, if you would like to print debug prints
DEBUG_PRINT=0

# define the location of bootloder where all necessary configs are defined.
BOOTLOADER_PATH=../bootloader_cm0p

# Include the flash map configuration and make configs
include $(BOOTLOADER_PATH)/shared_config.mk

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
COMPONENTS=FREERTOS PSOC6HAL LWIP MBEDTLS SECURE_SOCKETS OTA_MQTT

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

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

# Version of the app
APP_VERSION_MAJOR?=1
APP_VERSION_MINOR?=0
APP_VERSION_BUILD?=0

# Path of the key file passed as a parameter to the Python module "imgtool" for
# signing the image. Name of the key is file is set in bootloader_cm0p/shared_config.mk
# as a common parameter to all three applications
KEY_FILE_PATH=$(BOOTLOADER_PATH)/keys
###########################################################################
# Defines for anycloud-ota with MCUBOOT
#
# Additional / custom linker flags.
# This needs to be before finding LINKER_SCRIPT_WILDCARD as we need the extension defined
ifeq ($(TOOLCHAIN),GCC_ARM)

CY_ELF_TO_HEX_TOOL=$(CY_CROSSPATH)/bin/arm-none-eabi-objcopy
CY_ELF_TO_HEX_OPTIONS=-O ihex
LDFLAGS+=-Wl,--defsym=CM0P_FLASH_SIZE=$(BOOTLOADER_APP_FLASH_SIZE)
LDFLAGS+=-Wl,--defsym=CM0P_RAM_SIZE=$(BOOTLOADER_APP_RAM_SIZE)
LDFLAGS+=-Wl,--defsym=CM4_FLASH_SIZE=$(MCUBOOT_SLOT_SIZE)
LDFLAGS+=-Wl,--defsym=CY_BOOT_PRIMARY_1_SIZE=$(MCUBOOT_SLOT_SIZE)
LDFLAGS+=-Wl,--defsym=CY_BOOT_SCRATCH_SIZE=$(MCUBOOT_SCRATCH_SIZE)
LDFLAGS+=-Wl,--defsym=MCUBOOT_HEADER_SIZE=$(MCUBOOT_HEADER_SIZE)
LDFLAGS+=-Wl,--defsym=CY_FACT_APP_SIZE=$(FACT_APP_SIZE)
LDFLAGS+=-Wl,--defsym=CY_INT_FLASH_SIZE=$(INT_FLASH_SIZE)
LDFLAGS+=-Wl,--defsym=CY_EXT_FLASH_SIZE=$(EXTERNAL_FLASH_SIZE)

#Linker flags
LDFLAGS+=-Wl,--undefined=uxTopUsedPriority

#Linker script
LINKER_SCRIPT=$(wildcard ./linker_script/TARGET_$(TARGET)/TOOLCHAIN_$(TOOLCHAIN)/*.ld)

else
$(error Only GCC_ARM is supported at this point)
endif #GCC_ARM

#Additional defines for the MCUBoot
DEFINES+=CY_BOOT_SCRATCH_SIZE=$(MCUBOOT_SCRATCH_SIZE) \
         CY_BOOT_BOOTLOADER_SIZE=$(BOOTLOADER_APP_FLASH_SIZE) \
         CY_BOOT_PRIMARY_1_START=$(BOOTLOADER_APP_FLASH_SIZE) \
         CY_BOOT_PRIMARY_1_SIZE=$(MCUBOOT_SLOT_SIZE) \
         CY_BOOT_SECONDARY_1_SIZE=$(MCUBOOT_SLOT_SIZE) \
         MCUBOOT_IMAGE_NUMBER=1

# Defines to place Secondary slot in External Flash just 
# after factory app image.
ifeq ($(USE_EXT_FLASH),1)
DEFINES+=CY_BOOT_USE_EXTERNAL_FLASH=1 \
         CY_BOOT_SECONDARY_1_START=$(FACT_APP_SIZE)
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
               -S $(MCUBOOT_SLOT_SIZE) -M $(MCUBOOT_MAX_IMG_SECTORS) --overwrite-only -R $(ERASED_VALUE) -k $(KEY_FILE_PATH)/$(SIGN_KEY_FILE).pem

# Python path definition
ifeq ($(OS),Windows_NT)
PYTHON_PATH?=python
else
PYTHON_PATH?=python3
endif

POSTBUILD=\
cp -f $(BINARY_OUT_PATH).hex $(BINARY_OUT_PATH)_raw.hex;\
$(CY_ELF_TO_HEX_TOOL) --change-addresses=$(HEADER_OFFSET) $(CY_ELF_TO_HEX_OPTIONS) $(BINARY_OUT_PATH).elf $(BINARY_OUT_PATH)_unsigned.hex;\
$(PYTHON_PATH) $(IMGTOOL_PATH) $(SIGN_ARGS) $(BINARY_OUT_PATH)_unsigned.hex $(BINARY_OUT_PATH).hex;\

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
CY_GETLIBS_SHARED_PATH=../../

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

# Absolute path to the compiler's "bin" directory.
#
# The default depends on the selected TOOLCHAIN (GCC_ARM uses the ModusToolbox
# IDE provided compiler by default).
CY_COMPILER_PATH=


# Locate ModusToolbox IDE helper tools folders in default installation
# locations for Windows, Linux, and macOS.
CY_WIN_HOME=$(subst \,/,$(USERPROFILE))
CY_TOOLS_PATHS ?= $(wildcard \
    $(CY_WIN_HOME)/ModusToolbox/tools_* \
    $(HOME)/ModusToolbox/tools_* \
    /Applications/ModusToolbox/tools_*)

# If you install ModusToolbox IDE in a custom location, add the path to its
# "tools_X.Y" folder (where X and Y are the version number of the tools
# folder). Make sure you use forward slashes.
CY_TOOLS_PATHS+=

# Default to the newest installed tools folder, or the users override (if it's
# found).
CY_TOOLS_DIR=$(lastword $(sort $(wildcard $(CY_TOOLS_PATHS))))

ifeq ($(CY_TOOLS_DIR),)
$(error Unable to find any of the available CY_TOOLS_PATHS -- $(CY_TOOLS_PATHS). On Windows, use forward slashes.)
endif

$(info Tools Directory: $(CY_TOOLS_DIR))

include $(CY_TOOLS_DIR)/make/start.mk