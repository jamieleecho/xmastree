#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <mvkit/mv_app.h>
#include "mvkit/mv_document.h"


static char message[128];


/* Derived from Google AI */
static int str_end_cmp(const char *str, const char *suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) {
        return 1;
    }
    return strcmp(str + str_len - suffix_len, suffix);
}


static void doc_ensure_extension(MVDocument *doc) {
    if (str_end_cmp(doc->path, doc->extension)) {
        size_t str_len = strlen(doc->path);
        size_t suffix_len = strlen(doc->extension);
        if ((str_len + suffix_len) < sizeof(doc->path)) {
            strncat(doc->path + str_len, doc->extension, sizeof(doc->path) - 1 - suffix_len);
        }
    }
}


void mv_document_init(MVDocument *doc,
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
    doc->file_backed = false;
    doc->default_path = default_path;
    doc->extension = extension;
    strncpy(doc->path, path ? path : default_path, sizeof(doc->path));
    doc->path[MV_PATH_MAX - 1] = 0;
    if (path) {
        mv_document_revert(doc);
    }
    doc_ensure_extension(doc);
    mv_undo_manager_init(&(doc->undo_manager));
}


bool mv_document_new(MVDocument *doc) {
    if (!doc->new_model) {
        return false;
    }

    if (mv_document_is_dirty(doc)) {
        if (mv_app_show_message_box("Save before starting\r\na new document?", MVMessageBoxType_YesNo) ==
             MVMessageBoxResult_Yes) {
            if (mv_document_save(doc)) {
                mv_app_show_message_box("New aborted.", MVMessageBoxType_Info);
                return false;
            }
        }
    }

    int err = doc->new_model(doc->model, doc->path);

    if (err) {
        sprintf(message, "Failed to create\r\ndocument.\r\nError = %d", err);
        mv_app_show_message_box(message, MVMessageBoxType_Error);
        return false;
    }
    strncpy(doc->path, doc->default_path, sizeof(doc->path));
    doc->path[MV_PATH_MAX - 1] = 0;
    doc_ensure_extension(doc);
    doc->file_backed = false;
    mv_undo_manager_reset(&(doc->undo_manager));
    mv_app_refresh_menubar();
    return true;
}


bool mv_document_open(MVDocument *doc) {
    if (!doc->open_model) {
        return false;
    }

    if (mv_document_is_dirty(doc)) {
        if (mv_app_show_message_box("Save before opening\r\na new document?", MVMessageBoxType_YesNo) ==
             MVMessageBoxResult_Yes) {
            if (mv_document_save(doc)) {
                mv_app_show_message_box("Open aborted.", MVMessageBoxType_Info);
                return false;
            }
        }
    }

    if (!mv_app_show_open_dialog(doc->path, doc->extension)) {
        return false;
    }

    doc_ensure_extension(doc);
    _cgfx_setgc(MV_OUTPATH, GRP_PTR, PTR_SLP);
    Flush();
    int err = doc->open_model(doc->model, doc->path);
    _cgfx_setgc(MV_OUTPATH, GRP_PTR, PTR_ARR);
    Flush();
    if (err) {
        sprintf(message, "Failed to load document.\r\nError = %d", err);
        mv_app_show_message_box(message, MVMessageBoxType_Error);
        doc->file_backed = false;
        return true;
    }
    mv_document_opened(doc);
    return true;
}


void mv_document_opened(MVDocument *doc) {
    doc->file_backed = true;
    mv_undo_manager_reset(&(doc->undo_manager));
    mv_app_refresh_menubar();
}


static error_code mv_document_save_internal(MVDocument *doc, bool force_overwrite) {
    doc_ensure_extension(doc);

    if (!force_overwrite) {
        int fd = open(doc->path, FAP_READ);
        if (fd >= 0) {
            close(fd);
            if (mv_app_show_message_box("Overwrite existing\r\nfile?", MVMessageBoxType_YesNo) == MVMessageBoxResult_No) {
                return 0;
            }
        }
    }

    _cgfx_setgc(MV_OUTPATH, GRP_PTR, PTR_SLP);
    Flush();
    int err = doc->save_model(doc->model, doc->path);
    _cgfx_setgc(MV_OUTPATH, GRP_PTR, PTR_ARR);
    Flush();
    if (err) {
        sprintf(message, "Failed to save document.\r\nError = %d", err);
        mv_app_show_message_box(message, MVMessageBoxType_Error);
        return err;
    }
    doc->file_backed = true;
    mv_undo_manager_reset_undo_marker(&(doc->undo_manager));
    mv_app_refresh_menubar();
    return 0;
}


void mv_document_revert(MVDocument *doc) {
    if (!doc->open_model || !doc->file_backed) {
        return;
    }

    if (mv_document_is_dirty(doc)) {
        char oldpath[MV_PATH_MAX];
        strncpy(oldpath, doc->path, sizeof(oldpath));
        oldpath[MV_PATH_MAX - 1] = 0;
        if (mv_app_show_message_box("Save before reverting\r\ndocument?", MVMessageBoxType_YesNo) ==
             MVMessageBoxResult_Yes) {
            if (mv_document_save_internal(doc, true)) {
                strcpy(doc->path, oldpath);
                mv_app_show_message_box("Revert aborted.", MVMessageBoxType_Info);
                return;
            }
        }
        strcpy(doc->path, oldpath);
        mv_undo_manager_reset(&(doc->undo_manager));
        mv_app_refresh_menubar();
    }
}


error_code mv_document_save_as(MVDocument *doc) {
    if (!doc->save_model) {
        return 0;
    }

    if (!mv_app_show_save_dialog(doc->path, doc->extension)) {
        return 0;
    }

    return mv_document_save_internal(doc, false);
}


error_code mv_document_save(MVDocument *doc) {
    if (!doc->save_model) {
        return 0;
    }

    if (!doc->file_backed) {
        return mv_document_save_as(doc);
    } else {
        return mv_document_save_internal(doc, true);
    }
}


void mv_document_make_change(MVDocument *doc, const MVUndoItem *undo_item) {
    if (mv_undo_manager_all_undone(&(doc->undo_manager))) {
        mv_app_refresh_menubar();
    }
    mv_undo_manager_push_undo(&(doc->undo_manager), undo_item);
}


bool mv_document_is_dirty(const MVDocument *doc) {
    /* Possible optimizer error when we use ! instead of == */
    return mv_undo_manager_all_undone(&(doc->undo_manager)) == 0;
}


bool mv_document_can_new(const MVDocument *doc) {
    return doc->new_model != NULL;
}


bool mv_document_can_open(const MVDocument *doc) {
    return doc->open_model != NULL;
}


bool mv_document_can_revert(const MVDocument *doc) {
    return doc->open_model != NULL;
}


bool mv_document_can_save(const MVDocument *doc) {
    return doc->save_model != NULL;
}


bool mv_document_can_undo(const MVDocument *doc) {
    return mv_undo_manager_can_undo(&(doc->undo_manager));
}


bool mv_document_undo(MVDocument *doc) {
    if (mv_undo_manager_all_undone(&(doc->undo_manager))) {
        return false;
    }
    int val = mv_undo_manager_undo(&(doc->undo_manager));
    if (val || mv_undo_manager_all_undone(&(doc->undo_manager))) {
        mv_app_refresh_menubar();
    }
    return val;
}
