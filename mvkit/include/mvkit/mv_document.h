#ifndef _MVKIT_MV_DOCUMENT_H
#define _MVKIT_MV_DOCUMENT_H

#include <mvkit/mv_defs.h>          /* error_code, MV_PATH_MAX */
#include <mvkit/mv_undo_manager.h>  /* MVUndoManager, MVUndoItem, bool */

/*
 * A document wraps an application's data model with the standard file lifecycle
 * (new / open / save / revert), dirty tracking, an undo manager, and the
 * save-before-discarding prompts -- loosely analogous to AppKit's NSDocument.
 *
 * The app supplies an opaque `model` and three callbacks that act on it for a
 * given path; each returns 0 on success or an error code:
 *   new_model    initialize a fresh, empty model
 *   open_model   load the model from `path`
 *   save_model   write the model to `path`
 * Any callback may be NULL, which disables the corresponding operation (and the
 * matching mv_document_can_* query returns false).
 */
typedef struct {
    void *model;                                      /* app's data model (opaque to MVKit) */
    int (*new_model)(void *model, const char *path);  /* create empty model; NULL disables New */
    int (*open_model)(void *model, const char *path); /* load model from path; NULL disables Open */
    int (*save_model)(void *model, const char *path); /* save model to path; NULL disables Save */
    int file_backed;            /* nonzero once tied to a saved file on disk */
    const char *default_path;   /* path used for a new, unsaved document */
    const char *extension;      /* file extension incl. dot, e.g. ".xmt" (NULL for none) */
    char path[MV_PATH_MAX];     /* current file path */
    MVUndoManager undo_manager; /* per-document undo stack and dirty state */
} MVDocument;

/* Initialize `doc` with its model and callbacks. `extension` includes its dot
   (e.g. ".xmt"). If `path` is non-NULL the document opens it (and becomes
   file-backed); otherwise it starts as a new, unsaved document at
   `default_path`. The string arguments are borrowed and must outlive `doc`. */
extern void mv_document_init(MVDocument *doc,
                          const char *path,
                          const char *default_path,
                          const char *extension,
                          void *model,
                          int (*new_model)(void *model, const char *path),
                          int (*open_model)(void *model, const char *path),
                          int (*save_model)(void *model, const char *path));

/* Start a new, empty document, prompting to save first if dirty. Returns true
   if a new document was created, false if the operation was declined or failed. */
extern bool mv_document_new(MVDocument *doc);

/* Prompt to save if dirty, show the Open dialog, and load the chosen file.
   Returns true if the open flow ran (a file was chosen, even if loading then
   errored), false if the user cancelled. */
extern bool mv_document_open(MVDocument *doc);

/* Mark `doc` as successfully opened from its current path: set it file-backed,
   clear undo state, and refresh the menu bar. Use after loading a document
   outside the Open dialog (e.g. a path given on the command line). */
extern void mv_document_opened(MVDocument *doc);

/* Reload the document from disk, discarding unsaved changes (prompting to save
   first if dirty). No-op if the document is not file-backed. */
extern void mv_document_revert(MVDocument *doc);

/* Show the Save dialog and write the document to the chosen path. Returns 0 on
   success (or if cancelled) or an error code from the save callback. */
extern error_code mv_document_save_as(MVDocument *doc);

/* Save to the current path, or fall back to mv_document_save_as() if the
   document is not yet file-backed. Returns 0 on success or an error code. */
extern error_code mv_document_save(MVDocument *doc);

/* Record a model change and its corresponding undo action (pushed onto the
   document's undo manager), marking the document dirty. */
extern void mv_document_make_change(MVDocument *doc, const MVUndoItem *undo_item);

/* True if the document has unsaved changes. */
extern bool mv_document_is_dirty(const MVDocument *doc);

/* Capability queries, e.g. for enabling/disabling menu items. Each reflects
   whether the corresponding callback was supplied (and, for undo, whether there
   is anything to undo). */
extern bool mv_document_can_new(const MVDocument *doc);
extern bool mv_document_can_open(const MVDocument *doc);
extern bool mv_document_can_revert(const MVDocument *doc);
extern bool mv_document_can_save(const MVDocument *doc);
extern bool mv_document_can_undo(const MVDocument *doc);

/* Undo the most recent change. Returns true if an action was undone. */
extern bool mv_document_undo(MVDocument *doc);

#endif /* _MVKIT_MV_DOCUMENT_H */
