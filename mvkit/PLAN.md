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

> Historical note: this section captures the *original* scope. It has since been
> exceeded — `toolbox` was promoted into MVKit as `mv_image_grid` (Phase 5, with
> the `mv_view` protocol), and a foundational `mv_event` header was added. So
> `toolbox`/`stdbool.h` no longer live in the example app. See the phase sections
> below for what actually shipped.

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

### Phase 4 — Clean up app initialization
Today an app hand-writes a lot of wonky boilerplate: the `MIDSCR`/`MNDSCR` menu
tables, a verbose `WNDSCR mywindow = {…}` initializer, a `pre_init`, an `init`,
a `refresh_menus_action`, an event dispatcher, and a `main()` that wires them
together and handles `argc`/`argv`. Goal: collapse this so the common case is
near-zero boilerplate and `main()` is a single `mv_app_run(...)` call.

Status: **implemented.** Planned pieces (see resolutions at the end):

1. **`pre_init(argc, argv)` folded into `mv_app_run`.** Every app has a pre-init
   step (palette, images, model, and *optionally opening a document from
   `argv`*). Give it the signature `void pre_init(int argc, char **argv)` and
   have `mv_app_run` call it first, before window setup. `mv_app_run` returns
   `int` (the process exit code). Result: `main` collapses to
   `return mv_app_run(argc, argv, &window, pre_init, init, menu_actions,
   refresh_menus_action, event);` — the per-app `argc` checks and the
   "open the file named on the command line" logic move into the app's
   `pre_init`.

2. **Default no-op handlers** shipped by MVKit so a minimal app fills unused
   slots explicitly (clearer than `NULL`):
   `mv_app_pre_init_nop`, `mv_app_init_nop`, `mv_app_refresh_menus_action_nop`,
   `mv_app_event_nop`, and `mv_app_menu_actions_nop` (a sentinel-only action
   table). Keep `NULL` tolerated too, so both styles work.

3. **Menu/window boilerplate macros + `mv_menu_none`.** Replace the
   `WNDSCR`/`MNDSCR` hand-rolling with macros (working name `mv_set_menus`):
   one to declare a window from a menu array (filling the num-menus count and
   the default width/height/sync/reserved fields), and likely an `MV_MENU(...)`
   helper for each menu-table row. `mv_menu_none` gives a window with no menu
   bar for the trivial case.

The payoff — a do-nothing app becomes a few lines:
```c
mv_set_menus(window, "hello", mv_menu_none);
int main(int argc, char **argv) {
    return mv_app_run(argc, argv, &window,
        mv_app_pre_init_nop, mv_app_init_nop, mv_app_menu_actions_nop,
        mv_app_refresh_menus_action_nop, mv_app_event_nop);
}
```

Resolutions:
- **`mv_app_run` shape:** positional args + no-op defaults (chosen). cmoc has
  **no designated initializers**, which weakened the config-struct case; revisit
  `MVAppConfig` only if the arg list grows unwieldy.
- **Macros:** shipped `MV_MENU(...)` (a menus[] row) + `mv_set_menus(...)`
  (window from a menu array) + `mv_menu_none(...)` (no-menu window). cmoc accepts
  the initializer macros.
- **`mv_menu_none`:** declares `num_menus == 0` with a NULL menu pointer.
  Runtime-verified via the `examples/minimal` app (a framed, menu-less window
  draws and its close box works).
- **Default close-to-quit:** `mv_app_run` quits on the window close box
  (`MN_CLOS`) by default, so even an all-no-op app can be dismissed. An app
  pre-empts it by supplying its own `MN_CLOS` entry (e.g. xmastree's
  save-before-close).
- **Arg validation:** the app's `pre_init` owns it (xmastree's `argc > 2` check
  moved there); MVKit imposes no policy.
- **Window type:** the real entry is `mv_app_run_typed(window_type, ...)`;
  `mv_app_run` (WT_FWIN) and `mv_app_run_with_scrollbars` (WT_FSWIN) are
  zero-cost *macros* over it (not wrapper functions, which would leave a stack
  frame parked for the whole run since the loop never returns). Call sites are
  unchanged, so adding the option broke nothing.

### Phase 5 — `toolbox` as MVKit's first View
Status: **implemented.** Both toolbox and tree_view already shared one shape
(frame + draw + click handling), so this defined that as a protocol and shipped
the first concrete control:
- `mv_view` — the `MVView` protocol (frame, is_visible, draw/handle_click fn
  pointers) + helpers (`contains_point`, `draw`, `dispatch_click`). Concrete
  views embed it first.
- `mv_image_grid` — `MVImageGrid`, the generalized tool palette (variable item
  count / columns / size / colors; single-select), conforming to `MVView`.
  Replaced xmastree's hardcoded `ToolBox`.
- `TreeView` stays app-specific but now conforms to `MVView`.
- Event routing stays **manual**: the app keeps its own view list and calls
  `mv_view_dispatch_click` per view (a framework view hierarchy / responder
  chain is deferred — see below).

### Phase 6 — Upstream prep
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

- Whether `mv_app_run` takes an `MVAppConfig` struct (window, callbacks, and
  possibly name/palette/extension/colors) instead of positional params — now
  folded into **Phase 4** (app-init cleanup).
- Toolbox generalization shape — resolved in **Phase 5**: `MVImageGrid` with
  variable item count + columns, fixed (default 24x24) item size, single-select.
- A real view hierarchy / responder chain (the window owns views and routes
  events by hit-test) instead of the app's manual `mv_view_dispatch_click` loop —
  deferred; revisit once more than a couple of views per app is common.
