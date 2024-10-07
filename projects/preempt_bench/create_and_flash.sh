#!/bin/bash

ZEPHYR_IMG=build/zephyr/zephyr.img
ZEPHYR_BIN=build/zephyr/zephyr.bin
LOAD_ADDR=0x10000000
EMMC_LOAD=0x100000
SPL=/home/esteban/Documents/paris/khadas/fenix/build/u-boot/rk3588_spl_loader_v1.17.113.bin

function fill_clipboard() {
    if [ $XDG_SESSION_TYPE = "wayland" ] ; then
        wl-copy "mmc read \$pxefile_addr_r 0x100000 0x11b
        bootm start pxefile_addr_r
        bootm loados
        bootm go"
    else
        xclip -i "mmc read \$pxefile_addr_r 0x100000 0x11b
        bootm start pxefile_addr_r
        bootm loados
        bootm go"
    fi
}

function swap_config () {
    mv prj.conf temp
    mv other prj.conf
    mv temp other
}

function make_image () {
	rm -rf build
	west build -b khadas_edge2
	if [ $? -eq 1 ] ; then
		exit 1
	fi
	mkimage -C none -A arm64 -O linux -a $LOAD_ADDR -e $LOAD_ADDR -d $ZEPHYR_BIN $ZEPHYR_IMG
}

function prepare_board () {
	rkdeveloptool db $SPL
}

function flash_image () {
	rkdeveloptool wl $EMMC_LOAD $ZEPHYR_IMG
	rkdeveloptool rd
}

if [ ! -e "other" ]; then
    cp prj.conf other
    sed -i -e 's/CONFIG_EXPERIMENTAL_ASLR=n/CONFIG_EXPERIMENTAL_ASLR=y/g' other
fi

make_image

while : ; do
	rkdeveloptool ld 2>& 1 > /dev/null
	if [ $? -eq 1 ] ; then
		echo "Ensure that the device is in maskrom mode"
		read  -n 1 -p "Press any key to resume" _
	else
		break
	fi
done

prepare_board
flash_image

fill_clipboard

swap_config
make_image

while : ; do
	rkdeveloptool ld 2>& 1 > /dev/null
	if [ $? -eq 1 ] ; then
		echo "Ensure that the device is in maskrom mode"
		read  -n 1 -p "Press any key to resume" _
	else
		break
	fi
done

prepare_board
flash_image

fill_clipboard
