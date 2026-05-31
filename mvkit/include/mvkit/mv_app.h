#ifndef _MVKIT_MV_APP_H
#define _MVKIT_MV_APP_H

#include <stdlib.h>   /* size_t */
#include <cgfx.h>     /* WNDSCR, MSRET */
#include <stdbool.h>
#include <mvkit/mv_defs.h>
#include <mvkit/mv_event.h>   /* MVUiEvent */

/**
 * @file
 * The application object: palette/window setup, the main event loop, menu
 * dispatch, and the built-in modal dialogs (message box, file open/save).
 * Loosely analogous to AppKit's NSApplication.
 */

/** One row of the menu dispatch table passed to mv_app_run(): when the menu item
   (`menuid`, `itemno`) is chosen, `action` is called with the originating mouse
   record. Terminate the table with a sentinel whose menuid is < 0; that
   sentinel's `action` becomes the catch-all for unhandled selections. */
typedef struct {
    int menuid;
    int itemno;
    void (*action)(MSRET *msinfo, int menuid, int itemno);
} MVMenuItemAction;

/** Style of a message box, selecting which buttons are shown. Info/Warning/Error
   show a lone OK; OkCancel and YesNo show two buttons. */
typedef enum {
    MVMessageBoxType_Info,
    MVMessageBoxType_Warning,
    MVMessageBoxType_Error,
    MVMessageBoxType_OkCancel,
    MVMessageBoxType_YesNo
} MVMessageBoxType;

/** Which button dismissed a message box. Ok and Yes share value 0 (the primary/
   default button); Cancel and No are the secondary button. */
typedef enum {
    MVMessageBoxResult_Ok = 0,
    MVMessageBoxResult_Cancel,
    MVMessageBoxResult_Yes = 0,
    MVMessageBoxResult_No
} MVMessageBoxResult;

/** Install a screen palette: `palette[0..num_colors)` are cgfx color values for
   palette slots 0..num_colors-1. Call before mv_app_run(). */
extern void mv_app_init(const int *palette, size_t num_colors);

/** Open `mywindow`, enter the main event loop, and run until the program exits
   (in practice this does not return; the int return type lets main() be a
   single `return mv_app_run(...);`). Prefer the mv_app_run / mv_app_run_with_
   scrollbars macros below, which bake in the window type; call this directly
   only to pass another cgfx window type. `window_type` is a cgfx WT_ value
   (WT_FWIN = framed window, WT_FSWIN = framed window with scrollbars).
   Callbacks may be NULL, or pass the mv_app_*_nop defaults below. In order:
     pre_init              - run first, before window setup, with the process
                             argc/argv; do palette/image/model setup here, and
                             open a document named on the command line if wanted
     init                  - run once after the window is set up
     menu_actions          - menu dispatch table (see MVMenuItemAction). The
                             window close box quits the app by default; supply
                             an MN_CLOS entry to intercept it (e.g. save first)
     refresh_menus_action  - run before redrawing the menu bar; update item
                             enable/disable state here
     application_action    - called with each key press and content-area mouse
                             click for the app to handle */
extern int mv_app_run_typed(
    int window_type,
    int argc, char **argv,
    WNDSCR *mywindow,
    void (*pre_init)(int argc, char **argv),
    void (*init)(void),
    const MVMenuItemAction *menu_actions,
    void (*refresh_menus_action)(void),
    void (*application_action)(MVUiEvent *event)
);

/* Run an app in a plain framed window (WT_FWIN). Zero-cost convenience over
   mv_app_run_typed() -- a macro, so there is no wrapper stack frame (which,
   since mv_app_run_typed never returns, would otherwise persist for the whole
   run). */
#define mv_app_run(argc, argv, window, pre_init, init, menu_actions, refresh_menus_action, application_action) \
    mv_app_run_typed(WT_FWIN, (argc), (argv), (window), (pre_init), (init), \
                     (menu_actions), (refresh_menus_action), (application_action))

/* Run an app in a framed window with scrollbars (WT_FSWIN). */
#define mv_app_run_with_scrollbars(argc, argv, window, pre_init, init, menu_actions, refresh_menus_action, application_action) \
    mv_app_run_typed(WT_FSWIN, (argc), (argv), (window), (pre_init), (init), \
                     (menu_actions), (refresh_menus_action), (application_action))

/** Request that the menu bar be rebuilt before the next event is processed
   (triggers another call to the run loop's refresh_menus_action). Call after
   changing what menu items should be enabled. */
extern void mv_app_refresh_menubar(void);

/** Show a modal message box and return the button the user chose. `message` may
   embed CR/LF to wrap onto up to three lines; `type` selects the buttons. */
extern MVMessageBoxResult mv_app_show_message_box(const char *message, MVMessageBoxType type);

/** Show a modal Open file browser, centered on screen, listing files whose name
   ends with `ext` (including its dot, e.g. ".xmt"; NULL lists all). On confirm,
   copies the chosen name into `path` (a buffer of at least MV_PATH_MAX bytes)
   and returns it; returns NULL if cancelled. */
extern char *mv_app_show_open_dialog(char *path, const char *ext);

/** Like mv_app_show_open_dialog(), but a Save browser: it also offers a
   "[new file]" entry and appends `ext` to a typed name that lacks it. */
extern char *mv_app_show_save_dialog(char *path, const char *ext);


/* Default no-op handlers, so a minimal app can fill mv_app_run's callback
   slots explicitly instead of passing NULL. mv_app_menu_actions_nop is a
   sentinel-only dispatch table (every selection is ignored). */
extern void mv_app_pre_init_nop(int argc, char **argv);
extern void mv_app_init_nop(void);
extern void mv_app_refresh_menus_action_nop(void);
extern void mv_app_event_nop(MVUiEvent *event);
extern const MVMenuItemAction mv_app_menu_actions_nop[];


/* Default window geometry used by the menu/window macros below. */
#define MV_WINDOW_MIN_WIDTH  40
#define MV_WINDOW_MIN_HEIGHT 24
#define MV_WINDOW_SYNC       0xC0C0
#define MV_MENU_WIDTH        11

/* Declare one row of a MIDSCR items[] table: a menu item labelled `title`,
   initially enabled. Hides MIDSCR's trailing reserved bytes (always zero).
   Toggle enable/disable at runtime in the run loop's refresh_menus_action. */
#define MV_MENU_ITEM(title)          { (title), MN_ENBL, {0, 0, 0, 0, 0} }

/* Like MV_MENU_ITEM but initially disabled (greyed out). */
#define MV_MENU_ITEM_DISABLED(title) { (title), MN_DSBL, {0, 0, 0, 0, 0} }

/* A non-selectable separator row (a disabled dashed line). */
#define MV_MENU_SEPARATOR            { "----------", MN_DSBL, {0, 0, 0, 0, 0} }

/* Declare one row of a menus[] table: a menu titled `title` with id `menu_id`
   whose items are the static MIDSCR array `items`. */
#define MV_MENU(title, menu_id, items) \
    { (title), (menu_id), MV_MENU_WIDTH, sizeof(items) / sizeof((items)[0]), \
      MN_ENBL, {0, 0}, (items) }

/* Declare a window descriptor `var` titled `title`, driven by the static
   MNDSCR array `menus`. Fills in the menu count and default geometry. */
#define mv_set_menus(var, title, menus) \
    static WNDSCR var = { \
        (title), \
        sizeof(menus) / sizeof((menus)[0]), \
        MV_WINDOW_MIN_WIDTH, MV_WINDOW_MIN_HEIGHT, MV_WINDOW_SYNC, \
        {0, 0, 0, 0, 0, 0, 0}, \
        (menus) \
    }

/* Declare a window descriptor `var` titled `title` with no menu bar. */
#define mv_menu_none(var, title) \
    static WNDSCR var = { \
        (title), 0, \
        MV_WINDOW_MIN_WIDTH, MV_WINDOW_MIN_HEIGHT, MV_WINDOW_SYNC, \
        {0, 0, 0, 0, 0, 0, 0}, \
        (MNDSCR *)0 \
    }

#endif /* _MVKIT_MV_APP_H */
