# MVKit

An opinionated framework for building **Multi-Vue** applications on the Tandy
Color Computer 3 under NitrOS-9, loosely inspired by Apple's AppKit — for a much
lowlier 8-bit machine.

MVKit is being extracted from the [xmastree](../xmastree) example app, which
remains its living integration test. The long-term home for this code is a
dedicated section of [cmoc_os9](https://github.com/nitros9project/cmoc_os9); the
directory layout here is chosen to make that move a near-verbatim copy.

See [PLAN.md](PLAN.md) for the refactoring roadmap and status.

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

## Modules (target API)

| Module           | AppKit analog              | Status   |
|------------------|----------------------------|----------|
| `mv_undo_manager`| `NSUndoManager`            | planned  |
| `mv_image`       | `NSImage`                  | planned  |
| `mv_file_dialog` | `NSOpenPanel` / `NSSavePanel` | planned |
| `mv_document`    | `NSDocument`               | planned  |
| `mv_app`         | `NSApplication`            | planned  |

`mv_version` (current) is a Phase 0 placeholder proving the build plumbing.
