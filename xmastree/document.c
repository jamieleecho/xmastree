#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "document.h"


static char message[128];


/** Derived from Google AI */
int str_end_cmp(const char *str, const char *suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) {
        return 1;
    }
    return strcmp(str + str_len - suffix_len, suffix);
}


static void doc_ensure_extension(Document *doc) {
    if (str_end_cmp(doc->path, doc->extension)) {
        size_t str_len = strlen(doc->path);
        size_t suffix_len = strlen(doc->extension);
        if ((str_len + suffix_len) < sizeof(doc->path)) {
            strncat(doc->path + str_len, doc->extension, sizeof(doc->path) - 1 - suffix_len);
        }
    }
}


void document_init(Document *doc,
                   const char *path,
                   const char *default_path,
                   const char *extension,
                   void *model,
                   int (*new_model)(void *model, const char *path),
                   int (*open_model)(void *model, const char *path),
                   int (*save_model)(void *model, const char *path)) {
    doc->model = model;
    doc->new_model = new_model;
    doc->save_model = save_model;
    doc->open_model = open_model;
    doc->is_dirty = doc->file_backed = FALSE;
    doc->file_backed = FALSE;
    doc->default_path = default_path;
    doc->extension = extension;
    strncpy(doc->path, path ? path : default_path, sizeof(doc->path));
    doc->path[APP_PATH_MAX - 1] = 0;
    if (path) {
        document_revert(doc);
    }
    doc_ensure_extension(doc);
    undo_manager_init(&(doc->undo_manager));
}


void document_new(Document *doc) {
    if (!doc->open_model) {
        return;
    }

    if (doc->is_dirty) {
        if (show_message_box("Save before starting\r\na new document?", MessageBoxType_YesNo) ==
             MessageBoxResult_Yes) {
            document_save(doc);

            if (doc->is_dirty) {
                show_message_box("New aborted.", MessageBoxType_Info);
                return;
            }
        }
    }

    int err = doc->new_model(doc->model, doc->path);

    if (err) {
        sprintf(message, "Failed to create\r\ndocument.\r\nError = %d", err);
        show_message_box(message, MessageBoxType_Error);
        return;
    }
    strncpy(doc->path, doc->default_path, sizeof(doc->path));
    doc->path[APP_PATH_MAX - 1] = 0;
    doc_ensure_extension(doc);
    doc->file_backed = FALSE;
    doc->is_dirty = FALSE;
    undo_manager_reset(&(doc->undo_manager));
    app_refresh_menubar();
}


void document_open(Document *doc) {
    if (!doc->open_model) {
        return;
    }

    if (doc->is_dirty) {
        if (show_message_box("Save before opening\r\na new document?", MessageBoxType_YesNo) ==
             MessageBoxResult_Yes) {
            document_save(doc);

            if (doc->is_dirty) {
                show_message_box("Open aborted.", MessageBoxType_Info);
                return;
            }
        }
    }

    if (!show_open_dialog(doc->path)) {
        return;
    }

    doc_ensure_extension(doc);
    _cgfx_setgc(OUTPATH, GRP_PTR, PTR_SLP);
    Flush();
    int err = doc->open_model(doc->model, doc->path);
    _cgfx_setgc(OUTPATH, GRP_PTR, PTR_ARR);
    Flush();
    if (err) {
        sprintf(message, "Failed to load document.\r\nError = %d", err);
        show_message_box(message, MessageBoxType_Error);
        document_new(doc);
        return;
    }
    doc->file_backed = TRUE;
    doc->is_dirty = FALSE;
    undo_manager_reset(&(doc->undo_manager));
    app_refresh_menubar();
}


static int document_save_internal(Document *doc) {
    doc_ensure_extension(doc);
    int fd = open(doc->path, FAP_READ);
    if (fd >= 0) {
        close(fd);
        if (show_message_box("Overwrite existing\r\nfile?", MessageBoxType_YesNo) == MessageBoxResult_No) {
            return 0;
        }
    }

    _cgfx_setgc(OUTPATH, GRP_PTR, PTR_SLP);
    Flush();
    int err = doc->save_model(doc->model, doc->path);
    _cgfx_setgc(OUTPATH, GRP_PTR, PTR_ARR);
    Flush();
    if (err) {
        sprintf(message, "Failed to save document.\r\nError = %d", err);
        show_message_box(message, MessageBoxType_Error);
        return err;
    }
    doc->file_backed = TRUE;
    doc->is_dirty = FALSE;
    undo_manager_reset_undo_marker(&(doc->undo_manager));
    app_refresh_menubar();
    return 0;
}


void document_revert(Document *doc) {
    if (!doc->open_model || !doc->file_backed) {
        return;
    }

    if (doc->is_dirty) {
        char oldpath[APP_PATH_MAX];
        strncpy(oldpath, doc->path, sizeof(oldpath));
        oldpath[APP_PATH_MAX - 1] = 0;
        if (show_message_box("Save before reverting\r\ndocument?", MessageBoxType_YesNo) ==
             MessageBoxResult_Yes) {
            if (document_save_internal(doc)) {
                strcpy(doc->path, oldpath);
                show_message_box("Revert aborted.", MessageBoxType_Info);
                return;
            }
        }
        strcpy(doc->path, oldpath);
        undo_manager_reset(&(doc->undo_manager));
        app_refresh_menubar();
    }
}


int document_save_as(Document *doc) {
    if (!doc->save_model) {
        return 0;
    }

    if (!show_save_dialog(doc->path)) {
        return 0;
    }

    return document_save_internal(doc);
}


int document_save(Document *doc) {
    if (!doc->save_model) {
        return 0;
    }

    if (!doc->file_backed) {
        return document_save_as(doc);
    } else {
        return document_save_internal(doc);
    }
}


void document_set_dirty(Document *doc) {
    doc->is_dirty = TRUE;
    app_refresh_menubar();
}


void document_make_change(Document *doc, const UndoItem *undo_item) {
    document_set_dirty(doc);
    undo_manager_push_undo(&(doc->undo_manager), undo_item);
}


int document_is_dirty(const Document *doc) {
    return doc->is_dirty;
}


int document_can_new(const Document *doc) {
    return doc->new_model != NULL;
}


int document_can_open(const Document *doc) {
    return doc->open_model != NULL;
}


int document_can_revert(const Document *doc) {
    return doc->open_model != NULL;
}


int document_can_save(const Document *doc) {
    return doc->save_model != NULL;
}


int document_can_undo(const Document *doc) {
    return undo_manager_can_undo(&(doc->undo_manager));
}


int document_undo(Document *doc) {
    int val = undo_manager_undo(&(doc->undo_manager));
    if (val) {
        app_refresh_menubar();
    }
    return val;
}
