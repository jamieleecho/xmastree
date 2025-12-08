TARGET := xmastree
TARGET_DSK := ${TARGET}.dsk
CFILES := $(wildcard *.c)

MAME_DIR := ~/Applications/mame
MAME_ROM_PATH := $(MAME_DIR)/roms
EMULATED_SYSTEM := coco3
MAME := $(MAME_DIR)/mame
MAME_FLAGS := -speed 4 -window -ext:fdc:wd17xx:0 525qd
MAME_COMMAND := $(MAME) $(EMULATED_SYSTEM) -rompath $(MAME_ROM_PATH) $(MAME_FLAGS)

CC := cmoc
CMOC_OS9_DIR := cmoc_os9
CFLAGS := --os9 -I${CMOC_OS9_DIR}/include -I${CMOC_OS9_DIR}/cgfx/include
CMOC_OS9_LIBC_DIR := ${CMOC_OS9_DIR}/lib
CMOC_OS9_CGFX_DIR := ${CMOC_OS9_DIR}/cgfx

BASEIMAGE := disks/NOS9_6809_L2_v030300_coco3_80d.dsk
IMGTOOL_COPY := os9 copy
IMGTOOL_ATTR := os9 attr -e -pe -r -pe -npw

.PHONY := help libc libcgfx all clean run check-all check-lock check-lint \
		   utilities install-pre-commit lock run-tests sync fix-all fix-format fix-lint fix-lint-unsafe

all: ${TARGET_DSK}

${TARGET_DSK}: ${BASEIMAGE} ${TARGET}
	echo "Creating disk image $@ with program ${TARGET}"
	@head -c 2 ${BASEIMAGE} > $@_head.tmp
	@tail -c +3 ${BASEIMAGE} > $@.tmp  # Remove 2-byte header (start at char 3)
	@${IMGTOOL_COPY} ${TARGET} $@.tmp,CMDS/${TARGET}
	@${IMGTOOL_ATTR} $@.tmp,CMDS/${TARGET}
	@cat $@_head.tmp $@.tmp > $@
	@rm -f $@*.tmp  # Clean up temporary files

${TARGET}: libc libcgfx $(CFILES)
	$(CC) $(CFLAGS) -o $@ ${CFILES} -L${CMOC_OS9_LIBC_DIR} -L${CMOC_OS9_CGFX_DIR} -lc -lcgfx

libc:
	$(MAKE) -C ${CMOC_OS9_LIBC_DIR} all

libcgfx:
	$(MAKE) -C ${CMOC_OS9_CGFX_DIR} all

clean:
	$(MAKE) -C ${CMOC_OS9_LIBC_DIR} clean
	$(MAKE) -C ${CMOC_OS9_CGFX_DIR} clean
	rm -rf ${TARGET} ${TARGET_DSK}* cfg build .venv

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
