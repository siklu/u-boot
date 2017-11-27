#!/bin/bash
#
#
set -x

dd if=/dev/zero of=empty_head.dat  bs=1024  count=1

cat empty_head.dat u-boot-dtb.imx > /tftpboot/edwardk/nxp/u-boot-dtb.imx
