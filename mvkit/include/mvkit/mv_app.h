#ifndef _MVKIT_MV_APP_H
#define _MVKIT_MV_APP_H

#include <stdlib.h>   /* size_t */
#include <cgfx.h>     /* WNDSCR, MSRET */
#include <stdbool.h>
#include <mvkit/mv_defs.h>

/**
 * @file
 * The application object: palette/window setup, the main event loop, menu
 * dispatch, and the built-in modal dialogs (message box, file open/save).
 * Loosely analogous to AppKit's NSApplication.
 */

/** Which kind of input an MVUiEvent carries. */
typedef enum {
    MVUiEventType_KeyPress,
    MVUiEventType_MouseClick
} MVUiEventType;

/** Payload of a key-press event: the character read from the keyboard. */
typedef struct {
    char character;
} MVKeyEvent;

/** A UI event delivered to the application action callback. `event_type` selects
   the active member of `info`: a MVKeyEvent for key presses, or the cgfx MSRET
   mouse record for clicks. */
typedef struct {
    MVUiEventType event_type;
    union {
        MVKeyEvent key;
        MSRET mouse;
    } info;
} MVUiEvent;

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
   (this function does not return). The callbacks, any of which may be NULL:
     init                  - run once after the window is set up
     menu_actions          - menu dispatch table (see MVMenuItemAction)
     refresh_menus_action  - run before redrawing the menu bar; update item
                             enable/disable state here
     application_action    - called with each key press and content-area mouse
                             click for the app to handle */
extern void mv_app_run(
    WNDSCR *mywindow,
    void (*init)(void),
    const MVMenuItemAction *menu_actions,
    void (*refresh_menus_action)(void),
    void (*application_action)(MVUiEvent *event)
);

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

#endif /* _MVKIT_MV_APP_H */
