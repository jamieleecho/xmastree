# MVTheme — design

A theme is **the 16-entry color palette an MVKit app installs at startup**, with
documented, *enforced* roles for the low four registers. It exists because of a
hard constraint in the platform, described next.

## The constraint: chrome lives in palette registers 0–3

The OS-9 / NitrOS-9 CoCo3 window manager (`cowin`) draws **all** window chrome
itself, from a **hardcoded color table** — see
`nitros9/level2/coco3/modules/cowin.asm`:

```markdown
* Color table for 3D look stuff & others
* This should now match VIEW's color table (darkest to lightest for predictable
* brightness). Any color scheme following that rule shouldn't look bad.
WColor0  equ  0   black (Darkest)
WColor1  equ  1   dark grey (Dark)
WColor2  equ  2   light grey (Light)
WColor3  equ  3   (white) (Lightest)
```

Every chrome element pulls from these four fixed registers (verified in the draw
tables): the menu bar (`TopBar` → `WColor2/3/1`), dropdown menus, the `WT_DBOX`
**dialog double border** (`DBox` → `WColor1`), **shadows** (`WColor1`),
scrollbars and the `WT_FSWIN` 3D edges (`ScBar`/`FSWinTbl` → `WColor1/2/3`), and
single boxes (`SBox` → `WColor1/2`).

Two consequences:

1. **You cannot change *which* registers the chrome uses.** The assignment
   (0=darkest … 3=lightest) is compiled into the window manager. There is no API
   to point the chrome at other registers.
2. **You *can* change what those four registers look like** — their RGB — via
   the palette (`_cgfx_palette`, which `mv_app_init` already issues per slot).
   The window manager's own guidance: keep them **darkest → lightest** and "any
   color scheme following that rule shouldn't look bad."

Everything else an app might recolor (its own content, the colors MVKit passes
when *it* draws a dialog or a control) is ordinary palette use on registers
4–15, plus a couple of register-selection knobs.

> Experiments that produced "green chrome" were simply whatever RGB happened to
> sit in registers 0–3 at the time. xmastree's palette originally had register 1
> = white, scrambling the ramp; it has since been reordered to
> black / dark-grey / light-grey / white so its chrome renders correctly.

## The rule MVTheme enforces

> **Registers 0–3 are the window-chrome ramp and MUST be ordered
> darkest → lightest. Registers 4–15 are the app's to use freely.**

MVTheme makes this hard to get wrong by *naming* the four chrome slots (you
can't accidentally treat register 1 as "bright white") and by offering a
validity check.

## API (implemented)

`mvkit/include/mvkit/mv_theme.h`:

```c
typedef union {
    byte raw[16];                /* all 16 registers, flat (for the load loop) */
    struct {
      /* ===== Window-chrome ramp: palette registers 0-3 =====
        The OS-9 window manager hardcodes these for ALL chrome (menu bar,
        dropdowns, WT_DBOX dialog border, shadows, scrollbars, 3D edges). You
        choose only their RGB; the register assignment is fixed. The four values
        are cgfx color numbers (colno) and MUST run darkest -> lightest. */
      byte chrome_darkest;   /* register 0  -- e.g. black; window/content base */
      byte chrome_dark;      /* register 1  -- e.g. dark grey; shadows, dialog border */
      byte chrome_light;     /* register 2  -- e.g. light grey; menu bar, bars */
      byte chrome_lightest;  /* register 3  -- e.g. white; 3D highlights, text */

      /* ===== Content palette: registers 4-15 (cgfx colno), app's to define ===== */
      byte content[12];
    } ramp;
} MVTheme;

/* ===== Roles for MVKit-drawn UI (palette register indices 0-15) =====
    These pick WHICH registers MVKit uses when it draws its own widgets, so a
    theme can keep MVKit UI consistent with the chrome. Fixed, so macros. */
#define MV_THEME_DIALOG_FG     0
#define MV_THEME_DIALOG_BG     3
#define MV_THEME_CONTROL_FG    3
#define MV_THEME_CONTROL_BG    0
#define MV_THEME_WINDOW_BORDER 0

/* The canonical Multi-Vue look: black / dark-grey / light-grey / white ramp. */
extern const MVTheme mv_theme_default;

/* Install a theme: load all 16 palette registers (chrome 0-3 + content 4-15)
   and record the role selections for MVKit's own widgets to consult. Call once,
   in pre_init, before mv_app_run sets up the window. Supersedes mv_app_init's
   bare-palette form. */
extern void mv_app_set_theme(const MVTheme *theme);

/* True if chrome_darkest..chrome_lightest are non-decreasing in brightness.
   Intended for a debug/asserted check during development -- a theme that fails
   this will make the 3D chrome look wrong. */
extern bool mv_theme_chrome_ordered(const MVTheme *theme);
```

Notes:

- **cmoc has no C11 anonymous union members.** A bare anonymous `struct { … }`
  inside the union compiles but its fields are *not* injected into the union
  scope (`theme.chrome_darkest` fails to resolve), so the named slots live under
  a `.ramp` member: `theme.ramp.chrome_darkest`. The union is exactly 16 bytes,
  so `theme.raw[i]` aliases the matching ramp field — verified with a
  `sizeof(MVTheme) == 16` compile-time assert during implementation.
- **cmoc has no designated initializers**, so a theme literal is positional and
  initializes the union's first member, `raw[16]`; the ramp fields alias it:
  `static const MVTheme t = { { 0x00, 0x07, 0x38, 0x3f, /* …12 content… */ } };`.
- `mv_app_set_theme(&theme)` is the app's one palette call; it loads all 16
  registers via `_cgfx_palette`. `mv_app_init(palette, num_colors)` remains as
  the lower-level `int`-array form (not removed).
- MVKit widgets no longer hardcode colors: `mv_app_message_box` / the dialogs
  read `MV_THEME_DIALOG_FG`/`_BG` (via `MV_APP_FOREGROUND_COLOR`/`_BACKGROUND`,
  now aliases), and `MVImageGrid` reads `MV_THEME_CONTROL_FG`/`_BG` (via
  `MV_IMAGE_GRID_DEFAULT_FG`/`_BG`, now aliases). The role macros are the single
  source of truth. (Macros are `UPPER_CASE` to match MVKit convention; the
  earlier proposal used lowercase.)

## Theming in practice

- **Default look:** `mv_theme_default` is `black, dark-grey, light-grey, white`
  for 0–3 — the standard Multi-Vue chrome.
- **A tinted theme:** keep the brightness ramp but shift the hue, e.g. a blue
  scheme `darkest=navy, dark=blue, light=sky, lightest=white`. The chrome stays
  coherent because the ordering is preserved.
- **What you can't do:** make the menu bar a *different* register than the
  dialog border — they're both forced to the same `WColor*` slots. Theming is
  ramp-wide, not per-element, for the system chrome. (MVKit's *own* widgets can
  differ, via the role fields.)

## Validity

The single invariant — chrome 0–3 ordered darkest→lightest — is the whole
correctness story. Provide `mv_theme_chrome_ordered()` for a development-time
assert; do not spend runtime cost enforcing it in release builds (RAM/cycles are
scarce). Document the rule loudly at the struct and in `mv_theme_default`.

## Status / deferred

- **Implemented.** `mv_theme.h` defines `MVTheme`, the `MV_THEME_*` role macros,
  `mv_theme_default`, `mv_app_set_theme`, and `mv_theme_chrome_ordered`. Each
  function lives in its own translation unit (`mv_theme_default.c`,
  `mv_app_set_theme.c`, `mv_theme_chrome_ordered.c`) per the link-granularity
  rule. The dialog and grid color `#define`s now alias the role macros, and
  xmastree installs its colors via `mv_app_set_theme(&theme)` instead of a bare
  `int palette[]` — built and linked green.
- **The chrome-ramp contract was already in effect for xmastree** (done ahead of
  the `MVTheme` API): its palette registers 0–3 are
  black / dark-grey / light-grey / white, `MV_IMAGE_GRID_DEFAULT_FG` is the
  lightest (reg 3), and its AIF uses fg=reg 0 / bg=reg 3.
- **AIF colors.** The launcher's AIF carries the window's default fg/bg
  registers (xmastree now uses fg=`chrome_darkest` reg 0 / bg=`chrome_lightest`
  reg 3). Wiring the theme to AIF *generation* is a separate item.
- **Per-element chrome colors** are impossible (hardcoded), so not attempted.

## Resolved during implementation

- **`mv_app_set_theme` layers over `mv_app_init`** rather than replacing it: it
  loads the 16 `byte` registers from a theme; `mv_app_init` stays as the
  lower-level `int`-array form.
- **`content[12]` lives inside `MVTheme`.** Keeping all 16 registers in one
  union is what makes the `raw[16]` flat-load path (and its 16-byte aliasing)
  work; a separately-passed content array would lose that.
- **`mv_theme_default` is in its own `.o`** (`mv_theme_default.c`), so apps that
  define their own theme don't link the default's data.

## Open questions

- Wiring the theme to AIF *generation* (the launcher's default window fg/bg) is
  still a separate, unbuilt item — see "AIF colors" above.
- `mv_theme_chrome_ordered` is provided but not yet *called* anywhere; a
  debug-build assert in `mv_app_set_theme` could use it.
