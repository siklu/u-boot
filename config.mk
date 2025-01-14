#
# (C) Copyright 2000-2013
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
#########################################################################

# This file is included from ./Makefile and spl/Makefile.
# Clean the state to avoid the same flags added twice.
#
# (Tegra needs different flags for SPL.
#  That's the reason why this file must be included from spl/Makefile too.
#  If we did not have Tegra SoCs, build system would be much simpler...)
PLATFORM_RELFLAGS :=
PLATFORM_CPPFLAGS :=
PLATFORM_LDFLAGS :=
LDFLAGS :=
LDFLAGS_FINAL :=
OBJCOPYFLAGS :=
# clear VENDOR for tcsh
VENDOR :=
#########################################################################


#####################   SIKLU Additions  #####   edikk TBD. should be placed in platform.mk

# According to the requirements of PORTF-772,
# the u-boot version should be configured directly in file sdk_nxp/infra/u-boot/config.mk
# using hardcoded numbers.
# Do not add/remove white spaces, because
# UBOOT_MAJOR_STR, UBOOT_MINOR_STR, and UBOOT_BUILD_STR parsed in script ~/sdk_nxp/host-scripts/create_target_uboot.sh

UBOOT_MAJOR_STR = "21"
UBOOT_MINOR_STR = "1"
UBOOT_BUILD_STR = "0"

# For the following line to work when building U-Boot outside of the portfolio
# build system you need to export PROJECT_ROOT_DIR that points to a recently
# updated portfolio rep dir, or copy the portfolio repo tools/get_version.sh
# script to a "tools" dir in wherever your PROJECT_ROOT_DIR happnes to be.
SVNVERSION_STR=$(shell $(PROJECT_ROOT_DIR)/tools/get_version.sh $(shell pwd))
SIKLU_SVNVERSION=-DU_BOOT_SVNVERSION_STR=\""$(SVNVERSION_STR)"\"
$(info SVNVERSION_STR is "$(SVNVERSION_STR)")

SIKLU_FLAGS = $(SIKLU_SVNVERSION) -D_VER_MAJOR=\"$(UBOOT_MAJOR_STR)\" -D_VER_MINOR=\"$(UBOOT_MINOR_STR)\" -D_VER_BUILD=\"$(UBOOT_BUILD_STR)\"

PLATFORM_CPPFLAGS += $(SIKLU_FLAGS)
#########   End Siklu additions ################

ARCH := $(CONFIG_SYS_ARCH:"%"=%)
CPU := $(CONFIG_SYS_CPU:"%"=%)
ifdef CONFIG_SPL_BUILD
ifdef CONFIG_TEGRA
CPU := arm720t
endif
endif
BOARD := $(CONFIG_SYS_BOARD:"%"=%)
ifneq ($(CONFIG_SYS_VENDOR),)
VENDOR := $(CONFIG_SYS_VENDOR:"%"=%)
endif
ifneq ($(CONFIG_SYS_SOC),)
SOC := $(CONFIG_SYS_SOC:"%"=%)
endif

# Some architecture config.mk files need to know what CPUDIR is set to,
# so calculate CPUDIR before including ARCH/SOC/CPU config.mk files.
# Check if arch/$ARCH/cpu/$CPU exists, otherwise assume arch/$ARCH/cpu contains
# CPU-specific code.
CPUDIR=arch/$(ARCH)/cpu$(if $(CPU),/$(CPU),)

sinclude $(srctree)/arch/$(ARCH)/config.mk	# include architecture dependend rules
sinclude $(srctree)/$(CPUDIR)/config.mk		# include  CPU	specific rules

ifdef	SOC
sinclude $(srctree)/$(CPUDIR)/$(SOC)/config.mk	# include  SoC	specific rules
endif
ifneq ($(BOARD),)
ifdef	VENDOR
BOARDDIR = $(VENDOR)/$(BOARD)
else
BOARDDIR = $(BOARD)
endif
endif
ifdef	BOARD
sinclude $(srctree)/board/$(BOARDDIR)/config.mk	# include board specific rules
endif

ifdef FTRACE
PLATFORM_CPPFLAGS += -finstrument-functions -DFTRACE
endif

# Allow use of stdint.h if available
ifneq ($(USE_STDINT),)
PLATFORM_CPPFLAGS += -DCONFIG_USE_STDINT
endif

#########################################################################

RELFLAGS := $(PLATFORM_RELFLAGS)

PLATFORM_CPPFLAGS += $(RELFLAGS)
PLATFORM_CPPFLAGS += -pipe

LDFLAGS += $(PLATFORM_LDFLAGS)
LDFLAGS_FINAL += -Bstatic

export PLATFORM_CPPFLAGS
export RELFLAGS
export LDFLAGS_FINAL
export CONFIG_STANDALONE_LOAD_ADDR
