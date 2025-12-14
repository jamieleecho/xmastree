#ifndef _DOCUMENT_H
#define _DOCUMENT_H

#include "app.h"

typedef struct {
    void *model;
    int (*new_model)(void *model, const char *path);
    int (*open_model)(void *model, const char *path);
    int (*save_model)(void *model, const char *path);
    int is_dirty;
    int file_backed;
    const char *default_path;
    char path[APP_PATH_MAX];
} Document;

extern void document_init(Document *doc,
                          const char *path,
                          const char *default_path,
                          void *model,
                          int (*new_model)(void *model, const char *path),
                          int (*open_model)(void *model, const char *path),
                          int (*save_model)(void *model, const char *path));
extern void document_new(Document *doc);
extern void document_open(Document *doc);
extern void document_revert(Document *doc);
extern int document_save_as(Document *doc);
extern int document_save(Document *doc);
extern void document_set_dirty(Document *doc);
extern int document_is_dirty(const Document *doc);
extern int document_can_new(const Document *doc);
extern int document_can_open(const Document *doc);
extern int document_can_revert(const Document *doc);
extern int document_can_save(const Document *doc);

#endif