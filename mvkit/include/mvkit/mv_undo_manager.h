#ifndef _MVKIT_MV_UNDO_MANAGER_H
#define _MVKIT_MV_UNDO_MANAGER_H

#include <stdbool.h>

/* Capacity of the undo stack. Pushing past this drops the oldest action. */
#define MV_UNDO_MANAGER_MAX_UNDOS 16

/* A single reversible action: mv_undo_manager_undo() calls undo_function(object)
   to roll it back. The caller owns whatever `object` points to and must keep it
   valid until the action is undone or discarded. */
typedef struct {
    void (*undo_function)(void *object);
    void *object;
} MVUndoItem;

/* A bounded stack of MVUndoItems plus a "dirty" counter.

   `index` is the stack depth (how many actions can still be undone).
   `undo_marker` counts net changes since the last save baseline: push_undo
   increments it, undo decrements it, and it reads zero exactly when the document
   is back at its last saved/initialized state (see mv_undo_manager_all_undone).
   Treat the fields as private; use the functions below. */
typedef struct {
    MVUndoItem undo_items[MV_UNDO_MANAGER_MAX_UNDOS];
    int index;
    int undo_marker;
} MVUndoManager;

/* Initialize `undo_manager` to an empty, clean state. Call once before use. */
extern void mv_undo_manager_init(MVUndoManager *undo_manager);

/* Push `item` onto the undo stack (copied by value) and mark the document
   changed. If the stack is full, the oldest action is discarded to make room. */
extern void mv_undo_manager_push_undo(MVUndoManager *undo_manager, const MVUndoItem *item);

/* True if there is at least one action available to undo. */
extern bool mv_undo_manager_can_undo(const MVUndoManager *undo_manager);

/* Undo the most recent action: pop it and invoke its undo_function. Returns
   true if an action was undone, false if the stack was empty. */
extern bool mv_undo_manager_undo(MVUndoManager *undo_manager);

/* True when the document is at its saved/initialized baseline, i.e. no net
   unsaved changes. The inverse is what mv_document_is_dirty() reports. */
extern bool mv_undo_manager_all_undone(const MVUndoManager *undo_manager);

/* Mark the current state as the clean baseline (e.g. just after a save) WITHOUT
   clearing the undo stack, so prior actions remain undoable. */
extern void mv_undo_manager_reset_undo_marker(MVUndoManager *undo_manager);

/* Clear the undo stack and the dirty counter (e.g. on new/open). */
extern void mv_undo_manager_reset(MVUndoManager *undo_manager);

#endif /* _MVKIT_MV_UNDO_MANAGER_H */
