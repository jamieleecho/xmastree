# MVKit Refactoring Plan

Extract the reusable, app-agnostic core of **xmastree** into **MVKit** — an
opinionated framework for building Multi-Vue applications on the Tandy Color
Computer 3 under NitrOS-9, loosely inspired by Apple's AppKit.

This refactor happens **inside this repo** first. The end goal is to lift MVKit
into its own section of [cmoc_os9](https://github.com/nitros9project/cmoc_os9),
so every structural decision here is made to make that eventual move a
near-verbatim copy rather than a second refactor.

## Decisions (locked)

- **Namespace:** `mv_` for functions, `MV` for types (AppKit-style).
  e.g. `document_open()` → `mv_document_open()`, `Document` → `MVDocument`.
- **Packaging:** build MVKit into a static library `libmvkit.a`, linked with
  `-lmvkit`, exactly as the build already links `libc` / `libcgfx`.
- **Process:** plan-doc first (this file), then execute in phases below.

## Scope

### In MVKit (the framework)

| Current module  | MVKit module        | AppKit analog            | Notes |
|-----------------|---------------------|--------------------------|-------|
| `undo_manager`  | `mv_undo_manager`   | `NSUndoManager`          | Pure; zero dependencies. |
| `image`         | `mv_image`          | `NSImage`                | cgfx + app name via init. |
| `file_dialog`   | `mv_file_dialog`    | `NSOpen/SavePanel`       | cgfx only; self-contained. |
| `document`      | `mv_document`       | `NSDocument`             | Depends on undo + app types. |
| `app`           | `mv_app`            | `NSApplication`          | Run loop, menus, alerts, dialogs. |

### Stays in the example app (`xmastree/`)

`tree`, `tree_view`, `toolbox`, `xmastree.c`. The xmastree app is MVKit's
living integration test — it must compile and run after **every** commit.

`toolbox` is a borderline case: it is an app-shaped widget (hardcoded to 10
items) that could become MVKit's first real *View* class. It needs **design**,
not just relocation, so it is deferred to Phase 3.

## Coupling audit (current state)

The framework modules are already remarkably clean — the dependency arrows
already point the right way (app → framework, never the reverse). Findings:

- **Only one genuine app→framework leak:** `FILEDIALOG_EXT "xmt"` in `app.c:304`,
  consumed by `show_open_dialog` / `show_save_dialog`. `MVDocument` already
  carries an `extension` field — the dialogs should consume *that* instead.
- `image_init(app_name)` and `app_init(palette, num_colors)` already take their
  app-specifics as init parameters. Good — keep this pattern.
- `FOREGROUND_COLOR 0` / `BACKGROUND_COLOR 3` in `app.c` are framework *defaults*,
  not app data. They stay (can become configurable later if needed).

## Target layout

```
mvkit/
  include/mvkit/
    mvkit.h            umbrella header (includes the five below)
    mv_app.h
    mv_document.h
    mv_undo_manager.h
    mv_file_dialog.h
    mv_image.h
  src/
    mv_app.c
    mv_document.c
    mv_undo_manager.c
    mv_file_dialog.c
    mv_image.c
  Makefile             builds libmvkit.a
  README.md
  PLAN.md              (this file)

xmastree/              the example app
    xmastree.c  tree.c  tree.h  tree_view.c  tree_view.h
    toolbox.c   toolbox.h   stdbool.h   version.h
```

xmastree links MVKit via `-Imvkit/include -Lmvkit -lmvkit`, matching how it
already consumes cgfx.

## Guardrails (apply to every phase)

1. **Dependency rule is absolute:** MVKit never `#include`s an app header. All
   app-specifics enter through `*_init` / registration callbacks.
2. **Always green:** xmastree compiles after every commit; it is the integration
   test. Builds run in the `coco-dev` Docker image (cmoc / OS-9 tools live only
   there — see repo memory `build_toolchain`).
3. **One concern per commit** so each step is reviewable and revertable.
4. **Memory-frugal link granularity:** `lwlink` pulls static-archive members at
   *object-file* granularity — there is no function-level dead-code elimination.
   If `a()` and `b()` share a `.c`, referencing `a` drags `b` (and everything it
   calls) into the binary. So **split functions that are not always needed
   together into separate `.c` files**, one translation unit per independently
   usable feature, mirroring cmoc_os9's libc (e.g. `minmax.c` = `min`+`max`).
   Exception: functions that share file-scope `static` state are inherently one
   unit and must stay together (e.g. the run loop's signal/mouse statics).

   **Norm (as of the module-split pass):** default to *one public function per
   `.c`*. Group only trivial, always-co-used helpers (e.g. a cluster of
   one-line capability queries). When several functions share a helper or a
   `static`, externalize the shared piece through a private `src/`-only header
   (`mv_<module>_internal.h`) and give it its own `.c` so callers that don't
   need it don't link it — rather than bundling it with one caller. Worked
   examples: `mv_undo_manager` (7 files), `mv_document` (12 files, with
   `mv_document_internal.h`). Modules whose functions all share `static` state
   stay whole: `mv_image`, `mv_file_dialog`, `mv_app_run`.

## Phases

### Phase 0 — Build the seam (no logic moves)
- Create `mvkit/{include/mvkit,src}` + `mvkit/Makefile` producing `libmvkit.a`.
- Add umbrella header `mvkit/include/mvkit/mvkit.h`.
- Wire xmastree's Makefile: add `-Imvkit/include`, `-Lmvkit -lmvkit`, and a
  `libmvkit` build dependency mirroring the `libc` / `libcgfx` targets.
- **Verify the plumbing compiles before moving a single function** (an empty or
  stub lib is fine here).

### Phase 1 — Break the one coupling, in place
- Thread `MVDocument.extension` (still `Document.extension` at this point)
  through `show_open_dialog` / `show_save_dialog` so `FILEDIALOG_EXT` is gone.
- Self-contained commit; xmastree behavior unchanged; build green.

### Phase 2 — Migrate leaf-first, recompile after each move
Order chosen so each step compiles against already-moved dependencies:

1. `undo_manager` → `mv_undo_manager`   (zero deps)
2. `image`        → `mv_image`
3. `file_dialog`  → `mv_file_dialog`
4. `document`     → `mv_document`
5. `app`          → `mv_app`

Each step: relocate `.c`/`.h` into `mvkit/`, rename symbols to `mv_`/`MV`,
update xmastree's includes (or switch it to the umbrella `<mvkit/mvkit.h>`),
rebuild green, commit.

While migrating, **split each module along link-granularity lines** (guardrail 4):
one `.c` per independently usable feature so apps don't pay for what they don't
call. Candidate splits in the current code:
- `app` -> run loop + menubar (shares the signal/mouse statics) | message box |
  open/save dialog wrappers. An app using the run loop should not drag in the
  alert or file-dialog code if it never calls them.
- `image` -> load | draw | free, if apps commonly need a subset.
- `document` -> keep the cohesive `Document` core together, but new/open/save/
  revert/undo are split candidates if they pull in heavy dependencies.

A single public header per module (e.g. `mv_app.h`) can still front several
`.c` files — the header declares the API; the file split is purely about which
object members the linker can omit.

### Phase 3 — Document the public API
Add doc comments to every publicly consumable declaration in `mvkit/include/
mvkit/` — each exported function, type, struct field, and constant — so MVKit
reads as a documented framework before it grows or is upstreamed. Comments live
on the header declarations (the public contract); implementation files keep only
internal notes. No behavior change, so the build stays byte-identical.

### Phase 4 — (stretch) `toolbox` as MVKit's first View
Generalize the 10-item hardcode into a reusable view/control. This is design
work, not a lift — keep it out of the critical path until Phase 2 lands.

### Phase 5 — Upstream prep
Align `mvkit/Makefile` and layout with cmoc_os9's lib conventions; add tests
under cmoc_os9's `unittest/` harness. Then the directory copies up cleanly.

## Symbol rename reference (Phase 2)

Illustrative mapping; full list derived per-module during migration.

| Current                 | MVKit                      |
|-------------------------|----------------------------|
| `Document`              | `MVDocument`               |
| `document_init`         | `mv_document_init`         |
| `UndoManager` / `UndoItem` | `MVUndoManager` / `MVUndoItem` |
| `undo_manager_push_undo`| `mv_undo_manager_push_undo`|
| `app_init` / `run_application` | `mv_app_init` / `mv_app_run` |
| `show_message_box`      | `mv_app_show_message_box`  |
| `show_open_dialog` / `show_save_dialog` | `mv_app_show_open_dialog` / `mv_app_show_save_dialog` |
| `image_load_image`      | `mv_image_load`            |
| `app_file_dialog`       | `mv_file_dialog`           |

Enums/macros (`UiEvent`, `MessageBoxType`, `APP_PATH_MAX`, …) get `MV`/`MV_`
prefixes to match.

## Open questions for later

- Does `app_init` grow into an `MVAppConfig` struct (name, extension, palette,
  fg/bg colors) instead of positional params? Likely yes once `image_init`'s
  `app_name` and the dialog extension all want to live in one place.
- Toolbox generalization shape (Phase 3): item count, layout, and selection model.
