SOURCE := xmastree
SHORT_3_NAME := xmt

BUILD := build
ASSETS := assets
DEFAULT_PALETTE := ${ASSETS}/default-palette.txt
APP_PALLETTE := ${ASSETS}/app-palette.txt

TARGET := ${BUILD}/${SOURCE}
TARGET_IMAGES_DIR := ${BUILD}/images
TARGET_ICON := ${BUILD}/icon.${SHORT_3_NAME}
SOURCE_ICON := ${ASSETS}/app-icon.png
SYS_IMAGES_DIR := ${ASSETS}/sys-images
SOURCE_IMAGES := $(wildcard ${SYS_IMAGES_DIR}/*.png)
TARGET_IMAGES := $(addprefix ${TARGET_IMAGES_DIR}/, $(notdir $(SOURCE_IMAGES:.png=.i09))) $(addsuffix m.i09, $(addprefix ${TARGET_IMAGES_DIR}/, $(notdir $(basename ${SOURCE_IMAGES}))))
TARGET_SYS_IMAGES_DIR := SYS/${SOURCE}
TARGET_AIF := ${BUILD}/aif.${SHORT_3_NAME}
SOURCE_AIF := ${ASSETS}/$(notdir ${TARGET_AIF})
TARGET_DSK := ${TARGET}.os9
CFILES := $(wildcard ${SOURCE}/*.c)

MAME_DIR := ~/Applications/mame
MAME_ROM_PATH := $(MAME_DIR)/roms
EMULATED_SYSTEM := coco3
MAME := $(MAME_DIR)/mame
MAME_FLAGS := -speed 4 -window -cfg_directory ${ASSETS}/mame-cfgs -rompath $(MAME_ROM_PATH) -ext:fdc:wd17xx:0 525qd
MAME_COMMAND := $(MAME) $(EMULATED_SYSTEM) $(MAME_FLAGS)

CC := cmoc
CMOC_OS9_DIR := cmoc_os9
MVKIT_DIR := mvkit
MVKIT_LIB := ${MVKIT_DIR}/libmvkit.a
# MVKit needs no -I here: `libmvkit` installs it into cmoc's shared include dir,
# which cmoc auto-searches. cmoc_os9's libc/cgfx are still used in place via -I.
CFLAGS := --os9 -I${CMOC_OS9_DIR}/include -I${CMOC_OS9_DIR}/cgfx/include
CMOC_OS9_LIBC_DIR := ${CMOC_OS9_DIR}/lib
CMOC_OS9_CGFX_DIR := ${CMOC_OS9_DIR}/cgfx
CMOC_OS9_UTILS_DIR := ${CMOC_OS9_DIR}/utils
CMOC_OS9_UNITTEST_DIR := ${CMOC_OS9_DIR}/unittest

UEMACS_FILE := umacs
UEMACS_BINARY := ${CMOC_OS9_UTILS_DIR}/uemacs/${UEMACS_FILE}

MAZE_FILE := maze
MAZE_BINARY := ${CMOC_OS9_UNITTEST_DIR}/${MAZE_FILE}

BASEIMAGE := disks/NOS9_6809_L2_v030300_coco3_80d.os9
IMGTOOL_MAKDIR := os9 makdir
IMGTOOL_COPY := os9 copy
IMGTOOL_ATTR_EX := os9 attr -q -e -pe -r -pe -npw
IMGTOOL_ATTR_RO := os9 attr -q -r -ne -npe -npw


.PHONY: all clean help libc libcgfx libmvkit minimal mvkit-guide real-clean run run-minimal

## Build the OS-9 disk image (default target)
all: ${TARGET_DSK}

${TARGET_DSK}: ${BASEIMAGE} ${TARGET} ${TARGET_ICON} ${TARGET_AIF} ${TARGET_IMAGES} | ${CMOC_OS9_UTILS_DIR} ${CMOC_OS9_UNITTEST_DIR}
	echo "Creating disk image $@ with program ${TARGET}"
	@cp ${BASEIMAGE} $@
	@${IMGTOOL_MAKDIR} $@,CMDS/ICONS
	@${IMGTOOL_MAKDIR} $@,${TARGET_SYS_IMAGES_DIR}
	@${IMGTOOL_COPY} ${TARGET} $@,CMDS/$(notdir ${TARGET})
	@${IMGTOOL_ATTR_EX} $@,CMDS/$(notdir ${TARGET})
	@${IMGTOOL_COPY} ${TARGET_ICON} $@,CMDS/ICONS/$(notdir ${TARGET_ICON})
	@${IMGTOOL_ATTR_EX} $@,CMDS/ICONS/$(notdir ${TARGET_ICON})
	@${IMGTOOL_COPY} ${TARGET_AIF} $@,$(notdir ${TARGET_AIF})
	@${IMGTOOL_ATTR_RO} $@,$(notdir ${TARGET_AIF})
	@for each in ${TARGET_IMAGES}; do \
		${IMGTOOL_COPY} $${each} $@,${TARGET_SYS_IMAGES_DIR}/$$(basename $${each}); \
		${IMGTOOL_ATTR_RO} $@,${TARGET_SYS_IMAGES_DIR}/$$(basename $${each}); \
	done
	@${IMGTOOL_COPY} ${UEMACS_BINARY} $@,CMDS/$(notdir ${UEMACS_FILE})
	@${IMGTOOL_ATTR_EX} $@,CMDS/$(notdir ${UEMACS_FILE})
	@${IMGTOOL_COPY} ${MAZE_BINARY} $@,CMDS/$(notdir ${MAZE_FILE})
	@${IMGTOOL_ATTR_EX} $@,CMDS/$(notdir ${MAZE_FILE})

${BUILD}:
	mkdir -p ${BUILD}

${TARGET}: ${CFILES} | ${BUILD} libc libcgfx libmvkit
	$(CC) $(CFLAGS) -o $@ ${CFILES} -L${CMOC_OS9_LIBC_DIR} -L${CMOC_OS9_CGFX_DIR} -lmvkit -lc -lcgfx

${TARGET_ICON}: ${SOURCE_ICON} | ${BUILD}
	png-to-mvicon ${SOURCE_ICON} ${DEFAULT_PALETTE} $@

${TARGET_AIF}: ${SOURCE_AIF} | ${BUILD}
	@dos2unix -q -n ${SOURCE_AIF} $@
	@unix2mac -q $@

${TARGET_IMAGES_DIR}:
	mkdir -p ${TARGET_IMAGES_DIR}

${TARGET_IMAGES_DIR}/%.i09: ${SYS_IMAGES_DIR}/%.png ${APP_PALLETTE} | ${TARGET_IMAGES_DIR}
	png-to-os9-image $< ${APP_PALLETTE} $@

${TARGET_IMAGES_DIR}/%m.i09: ${SYS_IMAGES_DIR}/%.png | ${TARGET_IMAGES_DIR}
	png-to-os9-image --mask-index=0 $< ${APP_PALLETTE} $@

cmoc_os9:
	git clone https://github.com/nitros9project/cmoc_os9.git && \
	cd cmoc_os9 && \
	git checkout 102c2896f1255b38b1441e8904f1ba459882ab61

## Build the cmoc_os9 C library (libc)
libc: cmoc_os9
	$(MAKE) -C ${CMOC_OS9_LIBC_DIR} all

## Build the cmoc_os9 CoCo graphics library (libcgfx)
libcgfx: cmoc_os9
	$(MAKE) -C ${CMOC_OS9_CGFX_DIR} all

## Build and install the MVKit framework library into cmoc's shared dir
libmvkit: cmoc_os9
	$(MAKE) -C ${MVKIT_DIR} install

## Build the Using MVKit guide example apps (examples/guide/)
mvkit-guide: libc libcgfx libmvkit
	$(MAKE) -C ${MVKIT_DIR} guide

# Minimal MVKit example app (examples/minimal) + a bootable disk to test it.
MINIMAL_SRC := examples/minimal/minimal.c
MINIMAL_BIN := ${BUILD}/minimal
MINIMAL_ICON := ${BUILD}/icon.min
MINIMAL_AIF := ${BUILD}/aif.min
MINIMAL_DSK := ${BUILD}/minimal.os9

## Build the minimal MVKit example disk image (build/minimal.os9)
minimal: ${MINIMAL_DSK}

${MINIMAL_BIN}: ${MINIMAL_SRC} | ${BUILD} libc libcgfx libmvkit
	$(CC) $(CFLAGS) -o $@ ${MINIMAL_SRC} -L${CMOC_OS9_LIBC_DIR} -L${CMOC_OS9_CGFX_DIR} -lmvkit -lc -lcgfx

${MINIMAL_ICON}: ${SOURCE_ICON} ${DEFAULT_PALETTE} | ${BUILD}
	png-to-mvicon ${SOURCE_ICON} ${DEFAULT_PALETTE} $@

${MINIMAL_AIF}: ${ASSETS}/aif.min | ${BUILD}
	@dos2unix -q -n ${ASSETS}/aif.min $@
	@unix2mac -q $@

${MINIMAL_DSK}: ${BASEIMAGE} ${MINIMAL_BIN} ${MINIMAL_ICON} ${MINIMAL_AIF}
	echo "Creating minimal disk image $@"
	@cp ${BASEIMAGE} $@
	@${IMGTOOL_MAKDIR} $@,CMDS/ICONS
	@${IMGTOOL_COPY} ${MINIMAL_BIN} $@,CMDS/minimal
	@${IMGTOOL_ATTR_EX} $@,CMDS/minimal
	@${IMGTOOL_COPY} ${MINIMAL_ICON} $@,CMDS/ICONS/icon.min
	@${IMGTOOL_ATTR_EX} $@,CMDS/ICONS/icon.min
	@${IMGTOOL_COPY} ${MINIMAL_AIF} $@,aif.min
	@${IMGTOOL_ATTR_RO} $@,aif.min

## Build the utils
${CMOC_OS9_UTILS_DIR}: cmoc_os9
	$(MAKE) -C ${CMOC_OS9_UTILS_DIR} all

## Build the unittest
${CMOC_OS9_UNITTEST_DIR}: libc libcgfx
	cd ${CMOC_OS9_DIR} && $(MAKE) unittest-dsk all

## Remove build artifacts and the cmoc_os9 checkout
clean:
	@rm -rf ${TARGET} ${TARGET_DSK}* cfg build *.egg-info dist ${BUILD}
	@rm -f ${MVKIT_LIB} ${MVKIT_DIR}/src/*.o

## Remove everything clean removes, plus the cmoc_os9 checkout
real-clean: clean
	@rm -rf **/*~ ${CMOC_OS9_DIR}

## Show this help message
help:
	@awk 'BEGIN { \
		FS = ":"; \
		printf "Usage: make \033[36m<target>\033[0m\n\nTargets:\n"; \
	} \
	/^## / { doc = substr($$0, 4); next } \
	/^[a-zA-Z_][a-zA-Z0-9_-]*:/ { \
		if (doc) printf "  \033[36m%-20s\033[0m %s\n", $$1, doc; \
		doc = ""; next; \
	} \
	{ doc = "" }' $(MAKEFILE_LIST)

## Run the disk image in the MAME CoCo 3 emulator
run: ${TARGET_DSK}
	$(MAME_COMMAND) -flop1 ${TARGET_DSK}

## Run the minimal example disk image in the MAME CoCo 3 emulator
run-minimal: ${MINIMAL_DSK}
	$(MAME_COMMAND) -flop1 ${MINIMAL_DSK}
