TARGET := xmastree
TARGET_DSK = $(shell echo $(TARGET) | tr '[:lower:]' '[:upper:]').DSK
CFILES := $(wildcard *.c)

MAME_DIR := ~/Applications/mame
MAME_ROM_PATH := $(MAME_DIR)/roms
EMULATED_SYSTEM := coco3
MAME := $(MAME_DIR)/mame
MAME_FLAGS := -speed 4 -window -ext:fdc:wd17xx:0 525qd
MAME_COMMAND := $(MAME) $(EMULATED_SYSTEM) -rompath $(MAME_ROM_PATH) $(MAME_FLAGS)

CC := cmoc
CFLAGS := --os9 -Icmoc_os9/lib/include -Icmoc_os9/cgfx/include
CMOC_OS9_DIR := cmoc_os9
CMOC_OS9_LIBC_DIR := ${CMOC_OS9_DIR}/lib
CMOC_OS9_CGFX_DIR := ${CMOC_OS9_DIR}/cgfx

BASEIMAGE := disks/NOS9_6809_L2_v030300_coco3_80d.dsk
IMGTOOL_COPY := os9 copy
IMGTOOL_ATTR := os9 attr -e -pe -r -pe -npw

.PHONY: libc libcgfx all clean run

all: ${TARGET_DSK}

${TARGET_DSK}: ${BASEIMAGE} ${TARGET}
	echo "Creating disk image $@ with program ${TARGET}"
	head -c 2 ${BASEIMAGE} > $@_head.tmp
	tail -c +3 ${BASEIMAGE} > $@.tmp  # Remove 2-byte header (start at char 3)
	${IMGTOOL_COPY} ${TARGET} $@.tmp,CMDS/${TARGET}
	${IMGTOOL_ATTR} $@.tmp,CMDS/${TARGET}
	cat $@_head.tmp $@.tmp >> $@
	rm -f $@*.tmp  # Clean up temporary files

${TARGET}: libc libcgfx $(CFILES)
	$(CC) $(CFLAGS) -o $@ ${CFILES} -L${CMOC_OS9_LIBC_DIR} -L${CMOC_OS9_CGFX_DIR} -lc -lcgfx

libc:
	$(MAKE) -C ${CMOC_OS9_LIBC_DIR} all

libcgfx:
	$(MAKE) -C ${CMOC_OS9_CGFX_DIR} all

clean:
	$(MAKE) -C ${CMOC_OS9_LIBC_DIR} clean
	$(MAKE) -C ${CMOC_OS9_CGFX_DIR} clean
	rm -rf ${TARGET} ${TARGET_DSK}* cfg

run:
	$(MAME_COMMAND) -flop1 ${TARGET_DSK}
