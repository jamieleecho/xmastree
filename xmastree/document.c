#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "document.h"


static char message[128];


void document_init(Document *doc,
                   const char *path,
                   const char *default_path,
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
    default_path = default_path;
    strncpy(doc->path, path ? path : default_path, sizeof(doc->path));
    doc->path[APP_PATH_MAX - 1] = 0;
    if (path) {
        document_revert(doc);
    }
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
    doc->file_backed = FALSE;
    doc->is_dirty = FALSE;
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

    int err = doc->open_model(doc->model, doc->path);

    if (err) {
        sprintf(message, "Failed to load document.\r\nError = %d", err);
        show_message_box(message, MessageBoxType_Error);
        document_new(doc);
        return;
    }
    doc->file_backed = TRUE;
    doc->is_dirty = FALSE;
}


static int document_save_internal(Document *doc) {
    int err = doc->save_model(doc->model, doc->path);
    if (err) {
        sprintf(message, "Failed to save document.\r\nError = %d", err);
        show_message_box(message, MessageBoxType_Error);
        return err;
    }
    doc->file_backed = TRUE;
    doc->is_dirty = FALSE;
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
