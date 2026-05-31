# mvkit/app.mk -- reusable build for a Multi-Vue app built with MVKit.
#
# A consuming app's Makefile sets a few variables and includes this file:
#
#     APP  := myapp
#     SRCS := main.c
#     include /usr/local/share/cmoc/app.mk     # the installed location
#
# It produces a bootable OS-9 disk image, $(BUILD)/$(APP).os9, containing the
# program, its Multi-Vue launcher (icon + generated AIF), and any converted
# images. Run everything inside the coco-dev image (it has cmoc, lwtools,
# ToolShed and the PNG converters). Targets: all (default), run, clean.

ifndef APP
$(error set APP := <name> before including app.mk)
endif

# ---- inputs (override before the include as needed) -------------------------
SRCS         ?= $(wildcard *.c)
ASSETS       ?= assets
BUILD        ?= build

# OS-9 short name for the launcher files (aif.<SHORT> / icon.<SHORT>). Multi-Vue
# requires that extension be at most 3 letters; the executable keeps the full
# APP name. Defaults to the first three letters of APP -- override for a nicer
# short name (e.g. SHORT := xmt).
SHORT        ?= $(shell printf '%.3s' '$(APP)')

# Launcher: an icon PNG plus an AIF generated from the geometry/colors below.
APP_ICON     ?= $(ASSETS)/app-icon.png
ICON_PALETTE ?= $(ASSETS)/default-palette.txt
WIN_X        ?= 64
WIN_Y        ?= 8
WIN_W        ?= 40
WIN_H        ?= 25
WIN_FG       ?= 0          # default window foreground palette register
WIN_BG       ?= 3          # default window background palette register

# Image assets: PNGs in IMAGES_DIR become .i09 files under SYS/$(APP) on the
# disk (load them at run time with mv_image_load_resource()).
APP_PALETTE  ?= $(ASSETS)/app-palette.txt
IMAGES_DIR   ?= $(ASSETS)/sys-images

# Dependencies. MVKit is found automatically in cmoc's shared dir; cmoc_os9
# (libc/cgfx) and the NitrOS-9 base disk are still named explicitly. BASEIMAGE
# is required to build the disk image.
# make predefines CC=cc; default to cmoc only when CC wasn't set elsewhere
# (a command-line or app-Makefile CC still wins).
ifeq ($(origin CC),default)
CC := cmoc
endif
CMOC_OS9_DIR ?= cmoc_os9
CFLAGS       ?= --os9 -I$(CMOC_OS9_DIR)/include -I$(CMOC_OS9_DIR)/cgfx/include
LDLIBS       ?= -L$(CMOC_OS9_DIR)/lib -L$(CMOC_OS9_DIR)/cgfx -lmvkit -lc -lcgfx
BASEIMAGE    ?=

# MAME, for `make run` on the host (not in the container).
MAME_DIR     ?= ~/Applications/mame
MAME         ?= $(MAME_DIR)/mame
MAME_FLAGS   ?= -speed 4 -window -rompath $(MAME_DIR)/roms -ext:fdc:wd17xx:0 525qd \
                -autoboot_delay 3 -autoboot_command 'dos\n'

# ---- derived ----------------------------------------------------------------
BIN     := $(BUILD)/$(APP)
ICON    := $(BUILD)/icon.$(SHORT)
AIF     := $(BUILD)/aif.$(SHORT)
DSK     := $(BUILD)/$(APP).os9
IMGDIR  := $(BUILD)/images
SYSDIR  := SYS/$(APP)
PNGS    := $(wildcard $(IMAGES_DIR)/*.png)
IMGS    := $(addprefix $(IMGDIR)/,$(notdir $(PNGS:.png=.i09))) \
           $(addprefix $(IMGDIR)/,$(addsuffix m.i09,$(notdir $(basename $(PNGS)))))

# OS-9 file attributes: executable, and read-only data.
ATTR_EXEC := os9 attr -q -e -pe -r -pe -npw
ATTR_DATA := os9 attr -q -r -ne -npe -npw

.PHONY: all run clean

## Build the bootable disk image (default target)
all: $(DSK)

$(BUILD) $(IMGDIR):
	mkdir -p $@

$(BIN): $(SRCS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDLIBS)

$(ICON): $(APP_ICON) $(ICON_PALETTE) | $(BUILD)
	png-to-mvicon $(APP_ICON) $(ICON_PALETTE) $@

# Generate the Multi-Vue launcher AIF (CR line endings) from the variables.
$(AIF): | $(BUILD)
	@printf '%s\n\nICONS/icon.%s\n%s\n%s\n%s\n%s\n%s\n%s\n' \
		'$(APP)' '$(SHORT)' '$(WIN_X)' '$(WIN_Y)' '$(WIN_W)' '$(WIN_H)' '$(WIN_FG)' '$(WIN_BG)' > $@.tmp
	@unix2mac -q -n $@.tmp $@
	@rm -f $@.tmp

$(IMGDIR)/%.i09: $(IMAGES_DIR)/%.png $(APP_PALETTE) | $(IMGDIR)
	png-to-os9-image $< $(APP_PALETTE) $@

$(IMGDIR)/%m.i09: $(IMAGES_DIR)/%.png $(APP_PALETTE) | $(IMGDIR)
	png-to-os9-image --mask-index=0 $< $(APP_PALETTE) $@

$(DSK): $(BASEIMAGE) $(BIN) $(ICON) $(AIF) $(IMGS)
	@test -n "$(BASEIMAGE)" || { echo "set BASEIMAGE := <nitros9 base .os9 disk>"; exit 1; }
	cp $(BASEIMAGE) $@
	@os9 makdir $@,CMDS/ICONS
	@os9 makdir $@,$(SYSDIR)
	@os9 copy $(BIN) $@,CMDS/$(APP)
	@$(ATTR_EXEC) $@,CMDS/$(APP)
	@os9 copy $(ICON) $@,CMDS/ICONS/icon.$(SHORT)
	@$(ATTR_EXEC) $@,CMDS/ICONS/icon.$(SHORT)
	@os9 copy $(AIF) $@,aif.$(SHORT)
	@$(ATTR_DATA) $@,aif.$(SHORT)
	@for img in $(IMGS); do \
		os9 copy $$img $@,$(SYSDIR)/$$(basename $$img); \
		$(ATTR_DATA) $@,$(SYSDIR)/$$(basename $$img); \
	done
	@echo "Built $@"

## Launch the disk image in MAME (host-side; needs a display + CoCo 3 ROMs)
run: $(DSK)
	$(MAME) coco3 $(MAME_FLAGS) -flop1 $(DSK)

## Remove the build directory
clean:
	rm -rf $(BUILD)
