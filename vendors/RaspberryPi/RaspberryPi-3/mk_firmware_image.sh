#!/bin/bash

#set -xv

IMAGE_FILE=$1
TARGET=$2
MODEL=$3

IMAGE_DIR=$(dirname $IMAGE_FILE)

if [ -z "$MODEL" ]; then
	MODEL="1"
fi

if [ -e $PWD/bootcode.bin ]; then
	WORKING_DIR=$PWD
elif [ -d $PWD/vendors ]; then
	WORKING_DIR=$PWD/vendors/RaspberryPi/RaspberryPi-${MODEL}
else
	WORKING_DIR=$(dirname $0)
fi

if [ -z "$IMAGE_FILE" ]; then
	echo "FORMAT: $0 <image_file> <target>"
	echo "  where target is image or sdcard"
	exit 1
fi

which sfdisk >/dev/null 2>&1
CHECK=$?
if [ $CHECK -ne 0 ]; then
	echo "sfdisk missing, cannot continue."
	exit 1
fi

which stat >/dev/null 2>&1
CHECK=$?
if [ $CHECK -ne 0 ]; then
	echo "stat missing, cannot continue."
	exit 1
fi

if [ "$TARGET" = "sdcard" ] && [ ! -e ${IMAGE_DIR}/bootcode.bin ]; then
	if [ -e ~/.downloads/RaspberryPi_bootcode.bin ]; then
		cp ~/.downloads/RaspberryPi_bootcode.bin ${IMAGE_DIR}/bootcode.bin
	else
		wget https://github.com/raspberrypi/firmware/raw/stable/boot/bootcode.bin
		cp ${WORKING_DIR}/bootcode.bin ~/.downloads/RaspberryPi_bootcode.bin
		mv ${WORKING_DIR}/bootcode.bin ${IMAGE_DIR}/bootcode.bin
	fi
fi
if [ "$TARGET" = "sdcard" ] && [ ! -e ${IMAGE_DIR}/start.elf ]; then
	if [ -e ~/.downloads/RaspberryPi_start.elf ]; then
		cp ~/.downloads/RaspberryPi_start.elf ${IMAGE_DIR}/start.elf
	else
		wget https://github.com/raspberrypi/firmware/raw/stable/boot/start.elf
		cp ${WORKING_DIR}/start.elf ~/.downloads/RaspberryPi_start.elf
		mv ${WORKING_DIR}/start.elf ${IMAGE_DIR}/start.elf
	fi
fi
if [ ! -e ${IMAGE_DIR}/u-boot.bin ]; then
	if [ -e ${WORKING_DIR}/../../../boot/u-boot/u-boot.bin ]; then
		cp ${WORKING_DIR}/../../../boot/u-boot/u-boot.bin ${IMAGE_DIR}
	fi
fi

if [ "$TARGET" = "sdcard" ];then
	if [ ! -e ${IMAGE_DIR}/bootcode.bin ] || \
		[ ! -e ${IMAGE_DIR}/boot.scr ] || \
		[ ! -e ${IMAGE_DIR}/norm.scr ] || \
		[ ! -e ${IMAGE_DIR}/config.txt ] || \
		[ ! -e ${IMAGE_DIR}/start.elf ] || \
		[ ! -e ${IMAGE_DIR}/u-boot.bin ] || \
		[ ! -e ${IMAGE_DIR}/zImage ]; then
		echo "missing files required to make image, cannot continue!"
		ls -al ${IMAGE_DIR}
		exit 1
	fi
fi

if [ "$TARGET" = "image" ]; then
( \
	cd ${IMAGE_DIR}; \
	tar -czvf image.bin zImage rootfs.bin boot.scr norm.scr config.txt u-boot.bin *.dtb; \
)

elif [ "$TARGET" = "sdcard" ]; then

	# create empty disk image
	rm -f ${IMAGE_FILE}
	dd if=/dev/zero of=${IMAGE_FILE} bs=256M count=1
	ERR=$?
	[ $ERR -ne 0 ] && echo "dd err=$ERR, cannot continue!" && exit 1

	# create loopback device
	LOOPBACK=$(losetup -f)
	ERR=$?
	[ $ERR -ne 0 ] || [ -z "$LOOPBACK" ] && echo "losetup err=$ERR,cannot continue!" && exit 1

	# assign disk image to loopback device
	sudo losetup ${LOOPBACK} ${IMAGE_FILE}
	ERR=$?
	[ $ERR -ne 0 ] && echo "losetup err=$ERR, cannot continue!" && exit 1

	# create partitions
	(
		echo "start=2048, size=131072, type=c"
		echo "start=133120, size=92160, type=83"
		echo "start=225280, size=92160, type=83"
		echo "start=317440, size=204800, type=83"

	) | sudo sfdisk $LOOPBACK

	sync
	sudo losetup -d ${LOOPBACK}

	dd if=${IMAGE_FILE} of=${IMAGE_FILE}.p0 bs=512 count=2048
	dd if=${IMAGE_FILE} of=${IMAGE_FILE}.p1 bs=512 count=131072 skip=2048
	dd if=${IMAGE_FILE} of=${IMAGE_FILE}.p2 bs=512 count=92160 skip=133120
	dd if=${IMAGE_FILE} of=${IMAGE_FILE}.p3 bs=512 count=92160 skip=225280
	dd if=${IMAGE_FILE} of=${IMAGE_FILE}.p4 bs=512 count=204800 skip=317440

	mkfs.vfat -n boot ${IMAGE_FILE}.p1
	sudo losetup ${LOOPBACK} ${IMAGE_FILE}.p1
	mkdir -p ${IMAGE_DIR}/tmp
	sudo mount ${LOOPBACK} ${IMAGE_DIR}/tmp

	sudo cp -f ${IMAGE_DIR}/bootcode.bin \
		${IMAGE_DIR}/u-boot.bin \
		${IMAGE_DIR}/start.elf \
		${IMAGE_DIR}/config.txt \
		${IMAGE_DIR}/*.dtb \
		${IMAGE_DIR}/boot.scr \
		${IMAGE_DIR}/norm.scr ${IMAGE_DIR}/tmp
	sudo cp -f ${IMAGE_DIR}/zImage ${IMAGE_DIR}/tmp/zImageA
	sudo cp -f ${IMAGE_DIR}/zImage ${IMAGE_DIR}/tmp/zImageB

	sudo umount ${IMAGE_DIR}/tmp
	sync
	sudo losetup -d ${LOOPBACK}

	sz1=$(stat -c %s ${IMAGE_FILE}.p2)
	sz2=$(stat -c %s ${IMAGE_DIR}/rootfs.bin)
	padding=$(expr $sz1 - $sz2)

	cp -f ${IMAGE_DIR}/rootfs.bin ${IMAGE_FILE}.p2
	dd if=/dev/zero of=${IMAGE_FILE}.p2 bs=512 count=$(($padding / 512 )) seek=$(($sz2 / 512 ))
	cp -f ${IMAGE_FILE}.p2 ${IMAGE_FILE}.p3

	cat ${IMAGE_FILE}.p0 ${IMAGE_FILE}.p1 ${IMAGE_FILE}.p2 ${IMAGE_FILE}.p3 \
		${IMAGE_FILE}.p4 > ${IMAGE_FILE}
	rmdir ${IMAGE_DIR}/tmp
	rm -f ${IMAGE_FILE}.p*
fi
