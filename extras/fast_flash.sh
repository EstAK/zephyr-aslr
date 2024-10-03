#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"

ZEPHYR_IMG=build/zephyr/zephyr.img
ZEPHYR_BIN=build/zephyr/zephyr.bin
LOAD_ADDR=0x10000000
EMMC_LOAD=0x100000
SPL=extras/rk3588_spl_loader_v1.08.111.bin
RKDEVELOPTOOL=extras/rkdeveloptool/rkdeveloptool

function make_image () {
	west build -b khadas_edge2 samples/hello_world
	mkimage -C none -A arm64 -O linux -a "$LOAD_ADDR" -e "$LOAD_ADDR" -d "$ZEPHYR_BIN" "$ZEPHYR_IMG"
}

function prepare_board () {
	${RKDEVELOPTOOL} db $SPL
}

function flash_image () {
	${RKDEVELOPTOOL} wl $EMMC_LOAD $ZEPHYR_IMG
	${RKDEVELOPTOOL} rd
}

if [ "$SPL" = "SPL_LOCATION" ] ; then
	echo "Please set the location of your SPL in the script" 1>&2
	exit 1
fi
${RKDEVELOPTOOL} ld || (echo "Ensure that the device is in maskrom mode"; exit 1)

make_image
prepare_board
flash_image

CLIP_TOOL="xclip -sel c"
if [ "$XDG_SESSION_TYPE" = "wayland" ] ; then
	CLIP_TOOL="wl-copy"
fi
$CLIP_TOOL <<EOF
mmc read \$pxefile_addr_r 0x100000 0x1000
bootm start pxefile_addr_r
bootm loados
bootm go
EOF
