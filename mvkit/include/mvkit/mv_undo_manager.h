#ifndef _MVKIT_MV_UNDO_MANAGER_H
#define _MVKIT_MV_UNDO_MANAGER_H

#include <stdbool.h>

#define MV_UNDO_MANAGER_MAX_UNDOS 16

typedef struct {
    void (*undo_function)(void *object);
    void *object;
} MVUndoItem;

typedef struct {
    MVUndoItem undo_items[MV_UNDO_MANAGER_MAX_UNDOS];
    int index;
    int undo_marker;
} MVUndoManager;

extern void mv_undo_manager_init(MVUndoManager *undo_manager);
extern void mv_undo_manager_push_undo(MVUndoManager *undo_manager, const MVUndoItem *item);
extern bool mv_undo_manager_can_undo(const MVUndoManager *undo_manager);
extern bool mv_undo_manager_undo(MVUndoManager *undo_manager);
extern bool mv_undo_manager_all_undone(const MVUndoManager *undo_manager);
extern void mv_undo_manager_reset_undo_marker(MVUndoManager *undo_manager);
extern void mv_undo_manager_reset(MVUndoManager *undo_manager);

#endif /* _MVKIT_MV_UNDO_MANAGER_H */
