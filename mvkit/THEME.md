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

## Proposed API

`mvkit/include/mvkit/mv_theme.h`:

```c
typedef struct {
    /* ===== Window-chrome ramp: palette registers 0-3 =====
       The OS-9 window manager hardcodes these for ALL chrome (menu bar,
       dropdowns, WT_DBOX dialog border, shadows, scrollbars, 3D edges). You
       choose only their RGB; the register assignment is fixed. The four values
       are cgfx color numbers (colno) and MUST run darkest -> lightest. */
    int chrome_darkest;   /* register 0  -- e.g. black; window/content base */
    int chrome_dark;      /* register 1  -- e.g. dark grey; shadows, dialog border */
    int chrome_light;     /* register 2  -- e.g. light grey; menu bar, bars */
    int chrome_lightest;  /* register 3  -- e.g. white; 3D highlights, text */

    /* ===== Content palette: registers 4-15 (cgfx colno), app's to define ===== */
    int content[12];

    /* ===== Roles for MVKit-drawn UI (palette register indices 0-15) =====
       These pick WHICH registers MVKit uses when it draws its own widgets, so a
       theme can keep MVKit UI consistent with the chrome. */
    int dialog_fg;        /* message-box / dialog interior text   (default 0) */
    int dialog_bg;        /* message-box / dialog interior fill    (default 3) */
    int control_fg;       /* MVImageGrid border / highlight        (default 3) */
    int control_bg;       /* MVImageGrid background                 (default 0) */
    int window_border;    /* frame border register, _cgfx_border    (default 0) */
} MVTheme;

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

- **cmoc has no designated initializers**, so a theme literal is positional. The
  named struct fields are what carry the 0–3 guidance; field order is the ramp.
- `mv_app_set_theme` replaces today's `mv_app_init(palette, num_colors)` as the
  app's one palette call. `mv_app_init` can remain as a thin, lower-level form
  (set palette only) or be folded in — TBD during implementation.
- MVKit widgets stop hardcoding colors: `mv_app_message_box` / the dialogs read
  `dialog_fg/bg` (today `MV_APP_FOREGROUND_COLOR 0` / `..._BACKGROUND_COLOR 3`),
  and `MVImageGrid` reads `control_fg/bg` (today its `MV_IMAGE_GRID_DEFAULT_FG 1`
  / `..._BG 0`). Those `#define`s become the defaults inside `mv_theme_default`.

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

- **The chrome-ramp contract is already in effect for xmastree** (done ahead of
  the `MVTheme` API): its palette registers 0–3 were reordered to
  black / dark-grey / light-grey / white, `MV_IMAGE_GRID_DEFAULT_FG` was set to
  the lightest (reg 3), and its AIF uses fg=reg 0 / bg=reg 3. What remains
  unbuilt is the `MVTheme` struct/API itself (`mv_app_set_theme`,
  `mv_theme_default`, …) — this doc is still design-only.
- **AIF colors.** The launcher's AIF carries the window's default fg/bg
  registers (xmastree now uses fg=`chrome_darkest` reg 0 / bg=`chrome_lightest`
  reg 3). Wiring the theme to AIF *generation* is a separate item.
- **Per-element chrome colors** are impossible (hardcoded), so not attempted.

## Open questions

- Does `mv_app_set_theme` subsume `mv_app_init`, or layer over it?
- Is `content[12]` the right shape, or should content stay a caller-owned array
  passed separately (keeping `MVTheme` purely about roles + the ramp)?
- Should `mv_theme_default` live in its own `.o` so apps that define their own
  theme don't link it (link-granularity guardrail)?
