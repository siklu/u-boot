#!/bin/bash
# @(#) Convenience script for building Siklu portfolio NXP U-Boot
# For convenience and reference only, not used by sdk_nxp/platform.mk.

# Set PROJECT_ROOT_DIR to some place where you have checked out the portfolio
# repo so that you will have access to the scripts that are used in Siklu's
# additions to U-Boot. This variable does not affect the place where U-Boot is
# built, which is in the current directory where this script resides. [The
# requirement is to define this variable in the U-Boot directory is a Siklu
# design flaw that should be fixed.]
export PROJECT_ROOT_DIR="$(pwd)"

# This path for the cross compiler is the Siklu standard
export CROSS_COMPILE=/opt/arm/gcc-arm-10.2-2020.11-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-
export PATH=/opt/arm/gcc-arm-10.2-2020.11-x86_64-arm-none-linux-gnueabihf/bin/:${PATH}

# Temporarily comment out whatever you don't need.
#make oldconfig
make menuconfig
#make ARCH=arm CROSS_COMPILE=${CROSS_COMPILE} mx6ul_14x14_evk_defconfig

#make clean
#make
