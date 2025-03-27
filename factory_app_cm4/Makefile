################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Top-level application make file.
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

# Include the user configuration makefiles
include ../user_config.mk

################################################################################
# Basic Configuration
################################################################################

MTB_TYPE=PROJECT

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=factory_app_cm4

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

# Starting address of the CM4 app or the offset at which the header of an image
# will begin. Image = Header + App + TLV + Trailer. See MCUboot documentation for
# details.
# New relocated address = ORIGIN + HEADER_OFFSET
# ORIGIN is defined in the CM4 linker script and is usually the address next to
# the end of CM0+ image.
# Refer MCUBoot 'ExternalMemory.md' for details on the calculations
HEADER_OFFSET=0x7FE8000

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
COMPONENTS+=FREERTOS LWIP MBEDTLS SECURE_SOCKETS

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=CM0P_SLEEP CM0P_SECURE CM0P_CRYPTO CM0P_BLESS

# Set this application to run at the CM4
USER_APP_CORE=CM4
USER_APP_CORE_ID=0

CORE=$(USER_APP_CORE)
CORE_NAME=$(USER_APP_CORE)_$(USER_APP_CORE_ID)

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES=./configs

# Custom configuration of mbedtls library.
MBEDTLSFLAGS=MBEDTLS_USER_CONFIG_FILE='"mbedtls_user_config.h"'

# Add additional defines to the build process (without a leading -D).
DEFINES+=$(MBEDTLSFLAGS)
DEFINES+=CYBSP_WIFI_CAPABLE CY_RETARGET_IO_CONVERT_LF_TO_CRLF
DEFINES+=CY_RTOS_AWARE

# CY8CPROTO-062-4343W board shares the same GPIO for the user button (USER BTN1)
# and the CYW4343W host wake up pin. Since this example can use the GPIO for
# interfacing with the user button, the SDIO interrupt to wake up the host is
# disabled by setting CY_WIFI_HOST_WAKE_SW_FORCE to '0'.
#
# If you want the host wake up feature on CY8CPROTO-062-4343W board, change the GPIO pin
# for USER BTN in design/hardware & comment the below DEFINES line. For other
# targets commenting the below DEFINES line is sufficient.
ifeq ($(TARGET), $(filter $(TARGET), APP_CY8CPROTO-062-4343W APP_CY8CPROTO-062S2-43439))
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

# Additional / custom linker flags.
ifeq ($(TOOLCHAIN), GCC_ARM)
LDFLAGS=-Wl,--undefined=uxTopUsedPriority
CY_TOOLCHAIN_LS_EXT=ld
else
LDFLAGS=
$(error Selected toolchain is not supported at this moment)
endif

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

ifeq ($(OTA_SUPPORT),1)

# Enable MQTT Support
OTA_MQTT_SUPPORT=1

# Set OTA platform type (added to defines and used when finding the linker script)
OTA_PLATFORM=$(PLATFORM)

# Use the same json file for all application
OTA_FLASH_MAP=../flashmap/$(FLASH_MAP)

# To enable QSPI support
COMPONENTS+=OTA_PSOC_062

# Starting from the 4.0 version of the ota-update library, the library is now fully separated from the MCUBootloader.
# This means that it can function independently and work with any bootloader.
# This code example only supports MCUboot at this moment.
CY_BOOTLOADER=MCUBOOT
COMPONENTS+=$(CY_BOOTLOADER)

ifneq ($(CY_BOOTLOADER), MCUBOOT)
$(error This code example only supports MCUboot based bootloader at this moment)
endif

# Set the version of the app using the following three variables.
# This version information is passed to the Python module "imgtool" or "cysecuretools" while
# signing the image in the post build step. Default values are set as follows.
# Change the version here or over-ride by setting an environment variable
# before building the application.
#
# export APP_VERSION_MAJOR=2
#
APP_VERSION_MAJOR?=1
APP_VERSION_MINOR?=0
APP_VERSION_BUILD?=0


# Application version details
DEFINES+=OTA_SUPPORT=1\
         APP_VERSION_MAJOR=$(APP_VERSION_MAJOR)\
         APP_VERSION_MINOR=$(APP_VERSION_MINOR)\
         APP_VERSION_BUILD=$(APP_VERSION_BUILD)

# Add image type, version and core name metadata to image
IMG_VER_ARG=$(APP_VERSION_MAJOR).$(APP_VERSION_MINOR).$(APP_VERSION_BUILD)
DEFINES+=IMG_VER_MSG='"$(IMG_VER_ARG)"'
DEFINES+=IMG_TYPE_MSG='"$(IMG_TYPE)"'
DEFINES+=CORE_NAME_MSG='"$(CORE_NAME)"'

# Custom linker flags
LDFLAGS+=-Wl,--defsym=CM0P_FLASH_SIZE=$(FLASH_AREA_BOOTLOADER_SIZE)
LDFLAGS+=-Wl,--defsym=CM0P_RAM_SIZE=$(BOOTLOADER_APP_RAM_SIZE)
LDFLAGS+=-Wl,--defsym=CM4_FLASH_SIZE=$(FLASH_AREA_IMG_1_PRIMARY_SIZE)
LDFLAGS+=-Wl,--defsym=MCUBOOT_HEADER_SIZE=$(MCUBOOT_HEADER_SIZE)
LDFLAGS+=-Wl,--defsym=CY_FACT_APP_SIZE=$(FACT_APP_SIZE)
LDFLAGS+=-Wl,--defsym=CY_EXT_FLASH_SIZE=$(EXTERNAL_FLASH_SIZE)

# Add OTA_PLATFORM in DEFINES for platform-specific code
DEFINES+=$(PLATFORM)

# Python path for ota-bootloader-abstraction middlewares.
# Set Python path
ifeq ($(CY_PYTHON_PATH),)
ifeq ($(OS),Windows_NT)
CY_PYTHON_PATH?=$(shell which python)
else
CY_PYTHON_PATH?=$(shell which python3)
endif
endif # checking for Python path

# Build location local to this root directory.
CY_BUILD_LOCATION=./build

# Linker file path for ota-bootloader-abstraction middleware.
OTA_LINKER_FILE=$(wildcard ../bsps/TARGET_$(TARGET)/COMPONENT_$(CORE)/TOOLCHAIN_$(TOOLCHAIN)/linker$(LD_SUFFIX).$(CY_TOOLCHAIN_LS_EXT))
LINKER_SCRIPT=$(wildcard ../bsps/TARGET_$(TARGET)/COMPONENT_$(CORE)/TOOLCHAIN_$(TOOLCHAIN)/linker$(LD_SUFFIX).$(CY_TOOLCHAIN_LS_EXT))

# Include the ota_update.mk and mcuboot_support.mk makefiles from the middlewares.
ifneq ($(MAKECMDGOALS),getlibs)
ifneq ($(MAKECMDGOALS),get_app_info)
ifneq ($(MAKECMDGOALS),printlibs)
    include ../../mtb_shared/ota-update/*-v*/makefiles/ota_update.mk
    include ../../mtb_shared/ota-bootloader-abstraction/*-v*/makefiles/mcuboot/mcuboot_support.mk
endif
endif
endif

endif # OTA_SUPPORT

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

# Absolute path to the compiler's "bin" directory. The variable name depends on the
# toolchain used for the build. Refer to the ModusToolbox user guide to get the correct
# variable name for the toolchain used in your build.
#
# The default depends on the selected TOOLCHAIN (GCC_ARM uses the ModusToolbox
# software provided compiler by default).
CY_COMPILER_GCC_ARM_DIR=

include $(CY_TOOLS_DIR)/make/start.mk
