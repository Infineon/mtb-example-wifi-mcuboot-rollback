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

# Name of application (used to derive name of final linked file).
APPNAME=bootloader_cm0p

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

################################################################################
# MCUboot Specific Configuration
###############################################################################

# Include the application make files 
include ./shared_config.mk

# Set this to 1, if you would like to add external memory programming
# configurations to bootloader.
EN_XMEM_PROG ?= 1

CY_PYTHON_REQUIREMENT=true

.PHONY: generate_flashmap_cm0p

# Python command to generate flashmap header file from flashmap JSON file
ifneq ($(FLASH_MAP), )
generate_flashmap_cm0p:
	@echo -e "\n============================================================="
	@echo -e "= Generating cy_flash_map.h and flashmap.mk ="
	@echo -e "============================================================="
	$(CY_PYTHON_PATH) ./scripts/flashmap.py -p $(PLATFORM) -i ./flashmap/$(FLASH_MAP) -o ./source/cy_flash_map.h > ./flashmap.mk
	@echo -e "=============================================================\n"
    
-include ./flashmap.mk
endif

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

# Include the application make files 
include ./app.mk

# Do not define PSOC6HAL component as HAL is not supported for CM0+
COMPONENTS=

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

# Set this application to run at the CM0+
CORE=CM0P
CORE_NAME=CM0P_0

# Following define the flash map used by MCUBoot
DEFINES+=\
        CY_BOOT_BOOTLOADER_SIZE=$(BOOTLOADER_SIZE) \
        CY_BOOT_PRIMARY_1_SIZE=$(MCUBOOT_SLOT_SIZE) \
        CY_BOOT_SECONDARY_1_SIZE=$(MCUBOOT_SLOT_SIZE) \
        CY_BOOT_SCRATCH_SIZE=$(MCUBOOT_SCRATCH_SIZE) \
        CY_FACT_APP_SIZE=$(FACT_APP_SIZE)

# Add additional defines to the build process (without a leading -D).
DEFINES+=MBEDTLS_CONFIG_FILE='"mcuboot_crypto_config.h"' \
         ECC256_KEY_FILE='"$(SIGN_KEY_FILE).pub"' \
         MCUBOOT_IMAGE_NUMBER=$(MCUBOOT_IMAGE_NUMBER) \
         MCUBOOT_MAX_IMG_SECTORS=$(MCUBOOT_MAX_IMG_SECTORS) \
         MCUBOOT_LOG_LEVEL=MCUBOOT_LOG_LEVEL_INFO \
         $(FAMILY) \
         $(PLATFORM)

# Add defines to enable image overwrite operation
ifeq ($(USE_OVERWRITE), 1)
DEFINES+=MCUBOOT_OVERWRITE_ONLY
endif

# Add defines to enable usage of external flash for secondary images
# Defines to place Secondary slot in External Flash just 
# after factory app image.
ifeq ($(USE_EXTERNAL_FLASH), 1)
DEFINES+=CY_BOOT_USE_EXTERNAL_FLASH=1
DEFINES+=CY_MAX_EXT_FLASH_ERASE_SIZE=$(PLATFORM_CY_MAX_EXT_FLASH_ERASE_SIZE)
DEFINES+=CY_BOOT_EXTERNAL_FLASH_SECONDARY_1_OFFSET=$(FACT_APP_SIZE)
endif

# Enable external memory programming support.
ifeq ($(EN_XMEM_PROG), 1)
DEFINES+=CY_ENABLE_EXMEM_PROGRAM=1
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
LDFLAGS=

# Additional / custom libraries to link in to the application.
LDLIBS=


# Custom pre-build commands to run.
# MCUboot uses submodules and "make getlibs" currently does not support
# fetching submodules.
# The PREBUILD command fetches the submodules only if the content of the 
# "mcuboot/ext/mbedtls" directory is empty. 
# 
# See cy_serial_flash_prog.c file for details. 
PREBUILD_VAR=+\
@if [ -n "$$(ls -A $(MBEDTLS_PATH) 2>/dev/null)" ]; then\
    echo "Git submodules for MCUboot exist. Skipping this step...";\
    else\
    echo "Initializing Git Submodules for MCUboot";\
    cd libs/mcuboot;git submodule update --init --recursive;\
fi;\
$(MAKE) generate_flashmap_cm0p;\

PREBUILD=$(PREBUILD_VAR)

# Custom post-build commands to run.
POSTBUILD=

# Toolchain specific linker flags
ifeq ($(TOOLCHAIN), GCC_ARM)
LDFLAGS+=-Wl,--defsym=CM0P_FLASH_SIZE=$(BOOTLOADER_SIZE)
LDFLAGS+=-Wl,--defsym=CM0P_RAM_SIZE=$(BOOTLOADER_APP_RAM_SIZE)
LDFLAGS+=-Wl,--defsym=CY_FACT_APP_SIZE=$(FACT_APP_SIZE)
LDFLAGS+=-Wl,--defsym=CY_EXT_FLASH_SIZE=$(EXTERNAL_FLASH_SIZE)

else
$(error Only GCC_ARM is supported at this moment)
endif

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
