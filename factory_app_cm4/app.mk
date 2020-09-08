################################################################################
# \file app.mk
# \version 1.0
#
# \brief
# Configuration file for adding/removing source files. Modify this file
# to suit the application needs.
#
################################################################################
# \copyright
# Copyright 2020 Cypress Semiconductor Corporation
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

#mcuboot location
MCUBOOT_PATH=libs/mcuboot

################################################################################
# MCUBoot specific Files for app
################################################################################

MCUBOOT_CY_PATH=$(MCUBOOT_PATH)/boot/cypress
MCUBOOTAPP_PATH=$(MCUBOOT_CY_PATH)/MCUBootApp

SOURCES+=\
    $(wildcard $(MCUBOOT_PATH)/boot/bootutil/src/*.c)\
    $(wildcard $(MCUBOOT_CY_PATH)/cy_flash_pal/*.c)\

INCLUDES+=\
    ./keys\
    $(MCUBOOT_PATH)/boot/bootutil/include\
    $(MCUBOOT_PATH)/boot/bootutil/src\
    $(MCUBOOT_CY_PATH)/cy_flash_pal/include\
    $(MCUBOOT_CY_PATH)/cy_flash_pal/include/flash_map_backend\
    $(MCUBOOTAPP_PATH)\
    $(MCUBOOTAPP_PATH)/sysflash\
    $(MCUBOOTAPP_PATH)/config\
    $(MCUBOOTAPP_PATH)/config/mcboot_config\

################################################################################