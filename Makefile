SOURCE := xmastree
SHORT_3_NAME := xmt

BUILD := build
ASSETS := assets
DEFAULT_PALETTE := ${ASSETS}/default-palette.txt

TARGET := ${BUILD}/${SOURCE}
TARGET_ICON := ${BUILD}/icon.${SHORT_3_NAME}
SOURCE_ICON := ${ASSETS}/$(notdir ${TARGET}).png
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
IMGTOOL_ATTR_EX := os9 attr -e -pe -r -pe -npw
IMGTOOL_ATTR_RO := os9 attr -r -ne -npe -npw

.PHONY := help libc libcgfx all clean run check-all check-lock check-lint \
		   utilities install-pre-commit lock run-tests sync fix-all fix-format \
		   fix-lint fix-lint-unsafe assets

all: ${TARGET_DSK}

${TARGET_DSK}: ${BASEIMAGE} ${TARGET} ${TARGET_ICON} ${TARGET_AIF}
	echo "Creating disk image $@ with program ${TARGET}"
	@head -c 2 ${BASEIMAGE} > $@_head.tmp
	@tail -c +3 ${BASEIMAGE} > $@.tmp  # Remove 2-byte header (start at char 3)
	@${IMGTOOL_MAKDIR} $@.tmp,CMDS/ICONS
	@${IMGTOOL_COPY} ${TARGET} $@.tmp,CMDS/$(notdir ${TARGET})
	@${IMGTOOL_ATTR_EX} $@.tmp,CMDS/$(notdir ${TARGET})
	@${IMGTOOL_COPY} ${TARGET_ICON} $@.tmp,CMDS/ICONS/$(notdir ${TARGET_ICON})
	@${IMGTOOL_ATTR_EX} $@.tmp,CMDS/ICONS/$(notdir ${TARGET_ICON})
	@${IMGTOOL_COPY} ${TARGET_AIF} $@.tmp,$(notdir ${TARGET_AIF})
	@${IMGTOOL_ATTR_RO} $@.tmp,$(notdir ${TARGET_AIF})
	@cat $@_head.tmp $@.tmp > $@
	@rm -f $@*.tmp  # Clean up temporary files

${BUILD}:
	mkdir -p ${BUILD}

${TARGET}: libc libcgfx $(CFILES) ${BUILD}
	$(CC) $(CFLAGS) -o $@ ${CFILES} -L${CMOC_OS9_LIBC_DIR} -L${CMOC_OS9_CGFX_DIR} -lc -lcgfx

${TARGET_ICON}: ${SOURCE_ICON} ${BUILD} utilities
	uv run png-to-mvicon ${SOURCE_ICON} ${DEFAULT_PALETTE} $@

${TARGET_AIF}: ${SOURCE_AIF} ${BUILD}
	cp ${SOURCE_AIF} $@

libc:
	$(MAKE) -C ${CMOC_OS9_LIBC_DIR} all

libcgfx:
	$(MAKE) -C ${CMOC_OS9_CGFX_DIR} all

clean:
	@$(MAKE) -C ${CMOC_OS9_LIBC_DIR} clean
	@$(MAKE) -C ${CMOC_OS9_CGFX_DIR} clean
	@rm -rf ${TARGET} ${TARGET_DSK}* cfg build *.egg-info dist ${BUILD}

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

check-all: check-lock check-lint

check-lint: check-lock
	uv run ruff check

check-lock:
	uv lock --locked

utilities: check-lock sync
	uv pip install .

install-pre-commit:
	uv run pre-commit install

lock:
	uv lock

run:
	$(MAME_COMMAND) -flop1 ${TARGET_DSK}

run-tests: check-lock
	uv run pytest .

sync: check-lock
	uv sync --no-install-workspace
