#ifndef _MVKIT_MV_FILE_DIALOG_H
#define _MVKIT_MV_FILE_DIALOG_H

#include <cgfx.h>   /* path_id */

/**
 * MVKit's Save/Open file browser, cloned from cmoc_os9's cgfx MVFName() and
 * customized with confirm + Cancel buttons. `confirm_label` is the text on the
 * confirm button (e.g. "Open" or "Save"); a Cancel button is always shown.
 * Returns a pointer to the chosen filename, or NULL if cancelled.
 *
 * `allow_new` adds a "[new file]" entry that prompts for a name (use it for
 * Save, not Open) and makes the chosen name get `ext` appended if missing.
 * `ext` is an optional extension including its leading dot (e.g. ".xmt", NULL
 * for none); when set, the listing shows only matching files plus directories.
 */
char *mv_file_dialog(path_id path, const char *title, const char *confirm_label,
                     int allow_new, const char *ext,
                     int column, int row, int fg, int bg);

#endif /* _MVKIT_MV_FILE_DIALOG_H */
