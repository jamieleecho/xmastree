# Makefile to build the MVKit static library (libmvkit.a).
#
# MVKit is the reusable, app-agnostic core extracted from the xmastree example
# app. It links against cmoc_os9's libc/libcgfx exactly as the app does. The
# layout (include/mvkit + src) mirrors cgfx so the eventual lift into cmoc_os9
# is a near-verbatim copy. See PLAN.md.

CC := cmoc
AR := lwar
CMOC_OS9_DIR := ../cmoc_os9
CFLAGS := --os9 -Iinclude -I${CMOC_OS9_DIR}/include -I${CMOC_OS9_DIR}/cgfx/include

LIB := libmvkit.a

# Object modules that make up the library. Modules are added here as they are
# migrated out of xmastree (see PLAN.md, Phase 2).
MODS := src/mv_version.o src/mv_undo_manager.o src/mv_image.o

.PHONY: all clean help

## Build the MVKit static library (default target)
all: ${LIB}

${LIB}: ${MODS}
	${AR} -c $@ ${MODS}

src/%.o: src/%.c
	${CC} ${CFLAGS} -c -o $@ $<

## Remove build artifacts
clean:
	@rm -f src/*.o ${LIB}

## Show this help message
help:
	@awk 'BEGIN { FS = ":"; printf "Usage: make <target>\n\nTargets:\n"; } \
		/^## / { doc = substr($$0, 4); next } \
		/^[a-zA-Z_][a-zA-Z0-9_-]*:/ { if (doc) printf "  \033[36m%-16s\033[0m %s\n", $$1, doc; doc = ""; next } \
		{ doc = "" }' $(MAKEFILE_LIST)
