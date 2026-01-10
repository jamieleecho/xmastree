#ifndef _UNDO_MANAGER_H
#define _UNDO_MANAGER_H

#include <stdbool.h>

#define UNDO_MANAGER_MAX_UNDOS 16

typedef struct {
    void (*undo_function)(void *object);
    void *object;
} UndoItem;

typedef struct {
    UndoItem undo_items[UNDO_MANAGER_MAX_UNDOS];
    int index;
    int undo_marker;
} UndoManager;

extern void undo_manager_init(UndoManager *undo_manager);
extern void undo_manager_push_undo(UndoManager *undo_manager, const UndoItem *item);
extern bool undo_manager_can_undo(const UndoManager *undo_manager);
extern bool undo_manager_undo(UndoManager *undo_manager);
extern bool undo_manager_all_undone(const UndoManager *undo_manager);
extern void undo_manager_reset_undo_marker(UndoManager *undo_manager);
extern void undo_manager_reset(UndoManager *undo_manager);

#endif /* UNDO_MANAGER_H */