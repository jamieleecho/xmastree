#ifndef _MVKIT_MV_DOCUMENT_H
#define _MVKIT_MV_DOCUMENT_H

#include <mvkit/mv_defs.h>          /* error_code, MV_PATH_MAX */
#include <mvkit/mv_undo_manager.h>  /* MVUndoManager, MVUndoItem, bool */

typedef struct {
    void *model;
    int (*new_model)(void *model, const char *path);
    int (*open_model)(void *model, const char *path);
    int (*save_model)(void *model, const char *path);
    int file_backed;
    const char *default_path;
    const char *extension;
    char path[MV_PATH_MAX];
    MVUndoManager undo_manager;
} MVDocument;

extern void mv_document_init(MVDocument *doc,
                          const char *path,
                          const char *default_path,
                          const char *extension,
                          void *model,
                          int (*new_model)(void *model, const char *path),
                          int (*open_model)(void *model, const char *path),
                          int (*save_model)(void *model, const char *path));
extern bool mv_document_new(MVDocument *doc);
extern bool mv_document_open(MVDocument *doc);
extern void mv_document_opened(MVDocument *doc);
extern void mv_document_revert(MVDocument *doc);
extern error_code mv_document_save_as(MVDocument *doc);
extern error_code mv_document_save(MVDocument *doc);
extern void mv_document_make_change(MVDocument *doc, const MVUndoItem *undo_item);
extern bool mv_document_is_dirty(const MVDocument *doc);
extern bool mv_document_can_new(const MVDocument *doc);
extern bool mv_document_can_open(const MVDocument *doc);
extern bool mv_document_can_revert(const MVDocument *doc);
extern bool mv_document_can_save(const MVDocument *doc);
extern bool mv_document_can_undo(const MVDocument *doc);
extern bool mv_document_undo(MVDocument *doc);

#endif