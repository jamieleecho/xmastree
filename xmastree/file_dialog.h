#ifndef _FILE_DIALOG_H_
#define _FILE_DIALOG_H_

#include <cgfx.h>

/*
 * Clone of cmoc_os9's cgfx MVFName() file browser, customized into xmastree's
 * Save/Open dialogs. `confirm_label` is the text on the confirm button (e.g.
 * "Open" or "Save"); a Cancel button is always shown. Returns a pointer to the
 * chosen filename, or NULL if cancelled.
 *
 * `allow_new` adds a "[new file]" entry that prompts for a name (use it for
 * Save, not Open) and makes the chosen name get ".<ext>" appended if missing.
 * `ext` is an optional extension without the dot (e.g. "xmt", NULL for none);
 * when set, the listing shows only *.<ext> files plus directories.
 */
char *app_file_dialog(path_id path, const char *title, const char *confirm_label,
                      int allow_new, const char *ext,
                      int column, int row, int fg, int bg);

#endif /* _FILE_DIALOG_H_ */
