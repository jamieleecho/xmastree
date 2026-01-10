#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include "app.h"
#include "undo_manager.h"

typedef struct {
    void *model;
    int (*new_model)(void *model, const char *path);
    int (*open_model)(void *model, const char *path);
    int (*save_model)(void *model, const char *path);
    int file_backed;
    const char *default_path;
    const char *extension;
    char path[APP_PATH_MAX];
    UndoManager undo_manager;
} Document;

extern void document_init(Document *doc,
                          const char *path,
                          const char *default_path,
                          const char *extension,
                          void *model,
                          int (*new_model)(void *model, const char *path),
                          int (*open_model)(void *model, const char *path),
                          int (*save_model)(void *model, const char *path));
extern bool document_new(Document *doc);
extern bool document_open(Document *doc);
extern void document_revert(Document *doc);
extern error_code document_save_as(Document *doc);
extern error_code document_save(Document *doc);
extern void document_make_change(Document *doc, const UndoItem *undo_item);
extern bool document_is_dirty(const Document *doc);
extern bool document_can_new(const Document *doc);
extern bool document_can_open(const Document *doc);
extern bool document_can_revert(const Document *doc);
extern bool document_can_save(const Document *doc);
extern bool document_can_undo(const Document *doc);
extern bool document_undo(Document *doc);

#endif