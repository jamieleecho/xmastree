#ifndef _MVKIT_MV_MENU_H
#define _MVKIT_MV_MENU_H

#include <cgfx.h>     /* WNDSCR, MNDSCR, MIDSCR, MN_ENBL, MN_DSBL */

/**
 * @file
 * A small declarative DSL for cgfx menu-bar / window descriptors. These macros
 * hide the reserved bytes and positional layout of WNDSCR / MNDSCR / MIDSCR so
 * an app declares its menus and window readably:
 *
 *   static MIDSCR file_items[] = {
 *       MV_MENU_ITEM("New"), MV_MENU_SEPARATOR, MV_MENU_ITEM("Open..."),
 *   };
 *   static MNDSCR menus[] = { MV_MENU("File", MN_FILE, file_items), ... };
 *   mv_set_menus(mywindow, "myapp", menus);
 *
 * Convention: UPPER_CASE macros are value/initializer macros (use them inside
 * an array initializer); lower_case macros declare a static variable.
 */

/* Absolute minimum window size the window manager allows; a window's declared
   minimum must not go below these. */
#define MV_WINDOW_WIDTH_FLOOR   10
#define MV_WINDOW_HEIGHT_FLOOR  5

/* Default minimum window size, used by the no-suffix window macros below. */
#define MV_WINDOW_MIN_WIDTH  40
#define MV_WINDOW_MIN_HEIGHT 24
#define MV_WINDOW_SYNC       0xC0C0

/* The default dropdown width is the LENGTH of this dash string, so a menu's
   width and a separator's dashes share one source of truth and can't drift.
   cowin draws an item's title at its own length (it does NOT clip to the menu
   width), so a separator needs exactly as many dashes as the menu is wide. */
#define MV_MENU_WIDTH_OF(dashes) (sizeof(dashes) - 1)
#define MV_MENU_DASHES           "-----------"   /* 11 dashes */
#define MV_MENU_WIDTH            MV_MENU_WIDTH_OF(MV_MENU_DASHES)

/* Declare one row of a MIDSCR items[] table: a menu item labelled `title`,
   initially enabled. Hides MIDSCR's trailing reserved bytes (always zero).
   `title` is at most 14 chars -- MIDSCR's name field is char[15] and needs room
   for the NUL, so cmoc rejects anything longer at compile time. Toggle
   enable/disable at runtime with mv_menu_item_set_enabled(). */
#define MV_MENU_ITEM(title)          { (title), MN_ENBL, {0, 0, 0, 0, 0} }

/* Like MV_MENU_ITEM but initially disabled (greyed out). */
#define MV_MENU_ITEM_DISABLED(title) { (title), MN_DSBL, {0, 0, 0, 0, 0} }

/* A non-selectable separator row (a disabled dashed line). MV_MENU_SEPARATOR
   spans the default menu width; for a menu declared with MV_MENU_SIZED, use
   MV_MENU_SEPARATOR_S with a dash string of that width -- the string's length
   IS the width, so there is no per-width #if ladder. */
#define MV_MENU_SEPARATOR            MV_MENU_SEPARATOR_S(MV_MENU_DASHES)
#define MV_MENU_SEPARATOR_S(dashes)  { (dashes), MN_DSBL, {0, 0, 0, 0, 0} }

/* Enable or disable item `index` of a MIDSCR items[] array at runtime (e.g. in
   a refresh_menus_action), hiding the _mienbl field and the MN_ENBL/MN_DSBL
   encoding. `on` is any truthy/falsy value. */
#define mv_menu_item_set_enabled(items, index, on) \
    ((items)[index]._mienbl = (char)((on) ? MN_ENBL : MN_DSBL))

/* Declare one row of a menus[] table: a menu titled `title` with id `menu_id`
   whose items are the static MIDSCR array `items`. MV_MENU uses the default
   dropdown width; MV_MENU_SIZED takes an explicit `width` (the byte cowin uses
   to size the dropdown). To keep a custom-width menu's separators aligned,
   derive both the width and the separators from one dash string:

       #define EDIT_DASHES "--------------"
       static MIDSCR edit_items[] = {
           MV_MENU_ITEM("Undo"), MV_MENU_SEPARATOR_S(EDIT_DASHES), ...
       };
       ... MV_MENU_SIZED("Edit", MN_EDIT, MV_MENU_WIDTH_OF(EDIT_DASHES), edit_items) ... */
#define MV_MENU(title, menu_id, items) \
    MV_MENU_SIZED(title, menu_id, MV_MENU_WIDTH, items)
#define MV_MENU_SIZED(title, menu_id, width, items) \
    { (title), (menu_id), (width), sizeof(items) / sizeof((items)[0]), \
      MN_ENBL, {0, 0}, (items) }

/* Declare a window descriptor `var` titled `title`, driven by the static MNDSCR
   array `menus`. mv_set_menus uses the default minimum window size;
   mv_set_menus_sized takes an explicit minimum `width` x `height`. A size below
   MV_WINDOW_WIDTH_FLOOR x MV_WINDOW_HEIGHT_FLOOR fails to compile -- the
   companion typedef then has a negative length. */
#define mv_set_menus(var, title, menus) \
    mv_set_menus_sized(var, title, menus, MV_WINDOW_MIN_WIDTH, MV_WINDOW_MIN_HEIGHT)
#define mv_set_menus_sized(var, title, menus, width, height) \
    typedef char var##_mv_window_floor_ok[((width) >= MV_WINDOW_WIDTH_FLOOR && \
        (height) >= MV_WINDOW_HEIGHT_FLOOR) ? 1 : -1]; \
    static WNDSCR var = { \
        (title), \
        sizeof(menus) / sizeof((menus)[0]), \
        (width), (height), MV_WINDOW_SYNC, \
        {0, 0, 0, 0, 0, 0, 0}, \
        (menus) \
    }

/* Declare a window descriptor `var` titled `title` with no menu bar.
   mv_menu_none uses the default minimum window size; mv_menu_none_sized takes an
   explicit minimum `width` x `height` (same floor check as above). */
#define mv_menu_none(var, title) \
    mv_menu_none_sized(var, title, MV_WINDOW_MIN_WIDTH, MV_WINDOW_MIN_HEIGHT)
#define mv_menu_none_sized(var, title, width, height) \
    typedef char var##_mv_window_floor_ok[((width) >= MV_WINDOW_WIDTH_FLOOR && \
        (height) >= MV_WINDOW_HEIGHT_FLOOR) ? 1 : -1]; \
    static WNDSCR var = { \
        (title), 0, \
        (width), (height), MV_WINDOW_SYNC, \
        {0, 0, 0, 0, 0, 0, 0}, \
        (MNDSCR *)0 \
    }

#endif /* _MVKIT_MV_MENU_H */
