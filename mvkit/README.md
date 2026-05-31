# MVKit

An opinionated framework for building **Multi-Vue** applications on the Tandy
Color Computer 3 under NitrOS-9, loosely inspired by Apple's AppKit — for a much
lowlier 8-bit machine.

MVKit was extracted from the [xmastree](../xmastree) example app, which remains
its living integration test. The long-term home for this code is a dedicated
section of [cmoc_os9](https://github.com/nitros9project/cmoc_os9); the directory
layout here is chosen to make that move a near-verbatim copy.

See [PLAN.md](PLAN.md) for the roadmap and status, and [THEME.md](THEME.md) for
the theming/palette design.

## Layout

```
include/mvkit/   public headers; apps include <mvkit/mvkit.h>
src/             implementation
Makefile         builds libmvkit.a
```

## Building

The cmoc / OS-9 toolchain lives in the `jamieleecho/coco-dev` Docker image, not
on the host. From this directory inside the container:

```
make            # builds libmvkit.a
```

Apps link it with `-Imvkit/include -Lmvkit -lmvkit`, alongside `-lc -lcgfx`.

## Modules

| Module           | AppKit analog              | Notes |
|------------------|----------------------------|-------|
| `mv_undo_manager`| `NSUndoManager`            | one file per function |
| `mv_image`       | `NSImage`                  | |
| `mv_file_dialog` | `NSOpenPanel` / `NSSavePanel` | |
| `mv_app`         | `NSApplication`            | split: init / run / message_box / dialog / nop |
| `mv_document`    | `NSDocument`               | split per function |
| `mv_view`        | `NSView` (core)            | minimal view protocol (frame + draw + click) |
| `mv_image_grid`  | `NSMatrix`-ish             | single-select image-button grid (an `MVView`) |

Foundational headers (no AppKit analog): `mv_defs` (shared constants —
`MV_PATH_MAX`, `MV_INPATH`/`MV_OUTPATH`, the canonical `Flush()` declaration),
`mv_event` (`MVUiEvent` and friends, kept below `mv_app` so views don't depend
on it), and `mv_version` (framework version).

Include the whole API via `<mvkit/mvkit.h>`, or a single module via e.g.
`<mvkit/mv_document.h>`. See [THEME.md](THEME.md) for the palette/chrome rules.
