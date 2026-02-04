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
SOURCE_IMAGES :=  $(wildcard ${SYS_IMAGES_DIR}/*.png)
TARGET_IMAGES := $(addprefix ${TARGET_IMAGES_DIR}/, $(notdir $(SOURCE_IMAGES:.png=.i09))) $(addsuffix m.i09, $(addprefix ${TARGET_IMAGES_DIR}/, $(notdir $(basename ${SOURCE_IMAGES}))))
TARGET_SYS_IMAGES_DIR := SYS/${SOURCE}
TARGET_AIF := ${BUILD}/aif.${SHORT_3_NAME}
SOURCE_AIF := ${ASSETS}/$(notdir ${TARGET_AIF})
TARGET_DSK := ${TARGET}.dsk
CFILES := $(wildcard ${SOURCE}/*.c)

MAME_DIR := ~/Applications/mame
MAME_ROM_PATH := $(MAME_DIR)/roms
EMULATED_SYSTEM := coco3
MAME := $(MAME_DIR)/mame
MAME_FLAGS := -speed 4 -window -cfg_directory ${ASSETS}/mame-cfgs -rompath $(MAME_ROM_PATH) ${ASSETS }-ext:fdc:wd17xx:0 525qd
MAME_COMMAND := $(MAME) $(EMULATED_SYSTEM) $(MAME_FLAGS)

CC := cmoc
CMOC_OS9_DIR := cmoc_os9
CFLAGS := --os9 -I${CMOC_OS9_DIR}/include -I${CMOC_OS9_DIR}/cgfx/include
CMOC_OS9_LIBC_DIR := ${CMOC_OS9_DIR}/lib
CMOC_OS9_CGFX_DIR := ${CMOC_OS9_DIR}/cgfx

BASEIMAGE := disks/NOS9_6809_L2_v030300_coco3_80d.dsk
IMGTOOL_MAKDIR := os9 makdir
IMGTOOL_COPY := os9 copy
IMGTOOL_ATTR_EX := os9 attr -q -e -pe -r -pe -npw
IMGTOOL_ATTR_RO := os9 attr -q -r -ne -npe -npw

.PHONY := help libc libcgfx all clean run check-all check-lock check-lint \
		   install-pre-commit lock run-tests sync fix-all fix-format \
		   fix-lint fix-lint-unsafe assets

all: ${TARGET_DSK}

${TARGET_DSK}: ${BASEIMAGE} ${TARGET} ${TARGET_ICON} ${TARGET_AIF} ${TARGET_IMAGES}
	echo "Creating disk image $@ with program ${TARGET}"
	@head -c 2 ${BASEIMAGE} > $@_head.tmp
	@tail -c +3 ${BASEIMAGE} > $@.tmp  # Remove 2-byte header (start at char 3)
	@${IMGTOOL_MAKDIR} $@.tmp,CMDS/ICONS
	@${IMGTOOL_MAKDIR} $@.tmp,${TARGET_SYS_IMAGES_DIR}
	@${IMGTOOL_COPY} ${TARGET} $@.tmp,CMDS/$(notdir ${TARGET})
	@${IMGTOOL_ATTR_EX} $@.tmp,CMDS/$(notdir ${TARGET})
	@${IMGTOOL_COPY} ${TARGET_ICON} $@.tmp,CMDS/ICONS/$(notdir ${TARGET_ICON})
	@${IMGTOOL_ATTR_EX} $@.tmp,CMDS/ICONS/$(notdir ${TARGET_ICON})
	@${IMGTOOL_COPY} ${TARGET_AIF} $@.tmp,$(notdir ${TARGET_AIF})
	@${IMGTOOL_ATTR_RO} $@.tmp,$(notdir ${TARGET_AIF})
	echo ${TARGET_IMAGES}
	@for each in ${TARGET_IMAGES}; do \
		${IMGTOOL_COPY} $${each} $@.tmp,${TARGET_SYS_IMAGES_DIR}/$$(basename $${each}); \
		${IMGTOOL_ATTR_RO} $@.tmp,${TARGET_SYS_IMAGES_DIR}/$$(basename $${each}); \
	done
	@cat $@_head.tmp $@.tmp > $@
	@rm -f $@*.tmp  # Clean up temporary files


${BUILD}:
	mkdir -p ${BUILD}

${TARGET}: libc libcgfx $(CFILES) ${BUILD}
	$(CC) $(CFLAGS) -o $@ ${CFILES} -L${CMOC_OS9_LIBC_DIR} -L${CMOC_OS9_CGFX_DIR} -lc -lcgfx

${TARGET_ICON}: ${SOURCE_ICON} ${BUILD} utilities
	uv run png-to-mvicon ${SOURCE_ICON} ${DEFAULT_PALETTE} $@

${TARGET_AIF}: ${SOURCE_AIF} ${BUILD}
	@dos2unix -q -n ${SOURCE_AIF} $@
	@unix2mac -q $@

${TARGET_IMAGES_DIR}:
	mkdir -p ${TARGET_IMAGES_DIR}

${TARGET_IMAGES_DIR}/%.i09: ${SYS_IMAGES_DIR}/%.png utilities ${TARGET_IMAGES_DIR} ${APP_PALLETTE}
	uv run png-to-os9-image $< ${APP_PALLETTE} $@

${TARGET_IMAGES_DIR}/%m.i09: ${SYS_IMAGES_DIR}/%.png utilities ${TARGET_IMAGES_DIR}
	uv run png-to-os9-image --mask-index=0 $< ${APP_PALLETTE} $@

libc:
	$(MAKE) -C ${CMOC_OS9_LIBC_DIR} all

libcgfx:
	$(MAKE) -C ${CMOC_OS9_CGFX_DIR} all

clean:
	@$(MAKE) -C ${CMOC_OS9_LIBC_DIR} clean
	@$(MAKE) -C ${CMOC_OS9_CGFX_DIR} clean
	@rm -rf ${TARGET} ${TARGET_DSK}* cfg build *.egg-info dist ${BUILD} utilities

real-clean: clean
	@rm -rf .venv **/*~ **/__pycache__

help:
	@echo ${.PHONY}

fix-all: fix-format fix-lint lock

fix-format: check-lock
	uv run ruff format

fix-lint: check-lock
	uv run ruff check --fix

fix-lint-unsafe: check-lock
	uv run ruff check --fix --unsafe-fixes

check-all: check-lock check-lint check-types

check-lint: check-lock
	uv run ruff check

check-lock:
	uv lock --locked

.venv:
	uv venv

utilities: .venv
	uv pip install coco-tools==0.25
	touch utilities

install-pre-commit: .venv
	uv pip install pre-commit
	uv run pre-commit install

lock:
	uv lock

run:
	$(MAME_COMMAND) -flop1 ${TARGET_DSK}

run-tests: check-lock
	uv run coverage run -m pytest
	uv run coverage report --show-missing

sync: check-lock
	uv sync --no-install-workspace
