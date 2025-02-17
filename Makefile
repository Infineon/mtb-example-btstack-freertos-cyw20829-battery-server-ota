################################################################################
# \file Makefile
# \version 1.1
#
# \brief
# Bluetooth LE Battery Server OTA Application Makefile.
#
################################################################################
# \copyright
# Copyright 2018-2020 Cypress Semiconductor Corporation
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

# Type of ModusToolbox Makefile Options include:
#
# COMBINED    -- Top Level Makefile usually for single standalone application
# APPLICATION -- Top Level Makefile usually for multi project application
# PROJECT     -- Project Makefile under Application
#
MTB_TYPE=COMBINED
# Target board/hardware (BSP).
# To change the target, it is recommended to use the Library manager
# ('make modlibs' from command line), which will also update Eclipse IDE launch
# configurations. If TARGET is manually edited, ensure TARGET_<BSP>.mtb with a
# valid URL exists in the application, run 'make getlibs' to fetch BSP contents
# and update or regenerate launch configurations for your IDE.
TARGET=CYW920829M2EVK-02

# Core processor
CORE?=CM33

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=mtb-example-btstack-freertos-cyw20829-battery-server-ota

# Name of toolchain to use. Options include:
#
# GCC_ARM -- GCC 7.2.1, provided with ModusToolbox IDE
# ARM     -- ARM Compiler (must be installed separately)
# IAR     -- IAR Compiler (must be installed separately)
#
# See also: CY_COMPILER_PATH below
TOOLCHAIN=GCC_ARM

# Default build configuration. Options include:
#
# Debug   -- build with minimal optimizations, focus on debugging.
# Release -- build with full optimizations
# Custom -- build with custom configuration, set the optimization flag in CFLAGS
#
# If CONFIG is manually edited, ensure to update or regenerate launch configurations
# for your IDE.
CONFIG=Release

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

################################################################################
# Advanced Configuration
################################################################################

##############################
# COMPONENTS
##############################
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

COMPONENTS=FREERTOS WICED_BLE

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

##############################
# Defines/ Includes
##############################
# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES=./configs

CY_PYTHON_REQUIREMENT=true

# Python path definition
ifeq ($(OS),Windows_NT)
    CY_PYTHON_PATH?=python
else
    CY_PYTHON_PATH?=python3
endif

APP_VERSION_MAJOR?=1
APP_VERSION_MINOR?=1
APP_VERSION_BUILD?=0

# Add additional defines to the build process (without a leading -D).
DEFINES=CY_RETARGET_IO_CONVERT_LF_TO_CRLF CY_RTOS_AWARE STACK_INSIDE_FREE_RTOS

DEFINES+=\
        APP_VERSION_MAJOR=$(APP_VERSION_MAJOR)\
        APP_VERSION_MINOR=$(APP_VERSION_MINOR)\
        APP_VERSION_BUILD=$(APP_VERSION_BUILD)


##############################
# Floating point usage
##############################
# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT=

CY_BUILD_LOCATION:=./build


##############################
# Compiler and Linker Flags
##############################

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
ifeq ($(TOOLCHAIN),GCC_ARM)
    LDFLAGS=-Wl,--undefined=uxTopUsedPriority
else
    ifeq ($(TOOLCHAIN),IAR)
        LDFLAGS=--keep uxTopUsedPriority
    else
        ifeq ($(TOOLCHAIN),ARM)
            LDFLAGS=--undefined=uxTopUsedPriority
            LDFLAGS+=--diag_suppress=L6314W
        else
            LDFLAGS=
        endif
    endif
endif

# Additional / custom libraries to link in to the application.
LDLIBS=
# Custom pre-build commands to run.
PREBUILD=

CY_TOOLCHAIN_ARM_NOT_SUPPORTED = true
CY_TOOLCHAIN_IAR_NOT_SUPPORTED = true

# Custom post-build commands to run.
POSTBUILD=

################################################################################
################################################################################
# Basic OTA configuration
################################################################################
################################################################################

################################################################################
#
# OTA Build debugging
#
# OTA_BUILD_VERBOSE=1              Output info about Defines
# OTA_BUILD_FLASH_VERBOSE=1        Output info about Flash layout
# OTA_BUILD_COMPONENTS_VERBOSE=1   Output info about COMPONENTS, DEFINES, CY_IGNORE
# OTA_BUILD_DEFINES_VERBOSE=1      Output info about DEFINES
# OTA_BUILD_IGNORE_VERBOSE=1       Output info about CY_IGNORE
# OTA_BUILD_POST_VERBOSE=1         Output info about POSTBUILD values
#
#
###############################################################################
# Set to 1 to get more OTA Makefile / Build information for debugging build issues
OTA_BUILD_VERBOSE=

# Define to enable OTA logs
#DEFINES+=ENABLE_OTA_BOOTLOADER_ABSTRACTION_LOGS
#DEFINES+=ENABLE_OTA_LOGS
DEFINES+=OTA_USE_EXTERNAL_FLASH

OTA_SUPPORT = 1
OTA_BT_SUPPORT = 1
OTA_BT_SECURE = 0
OTA_BT_DEBUG = 0
ACTUAL_TARGET=$(subst APP_,,$(TARGET))

ifeq ($(ACTUAL_TARGET), CYW920829M2EVK-02)
    OTA_PLATFORM=CYW20829
    OTA_FLASH_MAP=./flash_map_json/cyw20829_xip_swap_single.json
    OTA_LINKER_FILE = ./template/TARGET_CYW920829M2EVK-02/COMPONENT_CM33/TOOLCHAIN_GCC_ARM/cyw20829_ns_flash_sahb_ota_xip.ld
    # For 20829 Secure Mode use "CY_DEVICE_LCS=SECURE" below
    #CY_DEVICE_LCS=SECURE
    ifeq ($(CY_DEVICE_LCS),SECURE)
	    CY_ENC_IMG?=1
	    DEFINES+=CY_ENC_IMG=1
	    CY_SMIF_ENC=1
	    ENABLE_ON_THE_FLY_ENCRYPTION=1
	    ifeq ($(ENABLE_ON_THE_FLY_ENCRYPTION),1)
	        DEFINES+= CY_XIP_SMIF_MODE_CHANGE=1 ENABLE_ON_THE_FLY_ENCRYPTION=1
	    endif
	    OTA_APP_POLICY_PATH?=policy/policy_secure.json
	    CY_NONCE?=MCUBootApp.signed_nonce.bin
	    CY_ENC_KEY?=keys/encrypt_key.bin
	    CY_OEM_PRIVATE_KEY?=keys/cypress-test-ec-p256.pem
	endif
endif

ifeq ($(ACTUAL_TARGET), CYW989829M2EVB-01)
    OTA_PLATFORM=CYW89829
    OTA_FLASH_MAP=./flash_map_json/cyw89829_xip_swap_single.json
    OTA_LINKER_FILE = ./template/TARGET_CYW989829M2EVB-01/COMPONENT_CM33/TOOLCHAIN_GCC_ARM/cyw89829_ns_flash_sahb_ota_xip.ld
endif

ifneq ($(MAKECMDGOALS),getlibs)
    ifneq ($(MAKECMDGOALS),get_app_info)
        ifneq ($(MAKECMDGOALS),printlibs)
            LIB_VER_NAME=$(shell cat ./deps/ota-update.mtb | awk -F\# '{print $$2}')
            include ../mtb_shared/ota-update/$(LIB_VER_NAME)/makefiles/ota_update.mk
            LIB_VER_NAME=$(shell cat ./deps/ota-bootloader-abstraction.mtb | awk -F\# '{print $$2}')
            include ../mtb_shared/ota-bootloader-abstraction/$(LIB_VER_NAME)/makefiles/mcuboot/mcuboot_support.mk
        endif
    endif
endif
include ./local.mk

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
CY_GETLIBS_SHARED_PATH=../

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
