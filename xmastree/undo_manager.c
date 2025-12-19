#include <stdio.h>
#include <string.h>
#include "undo_manager.h"


void undo_manager_init(UndoManager *undo_manager) {
    undo_manager_reset(undo_manager);
}


void undo_manager_push_undo(UndoManager *undo_manager, const UndoItem *item) {
    if (undo_manager->index >= (UNDO_MANAGER_MAX_UNDOS - 1)) {
        memcpy((char *)undo_manager->undo_items, (char *)(undo_manager->undo_items + 1), sizeof(undo_manager->undo_items) - sizeof(undo_manager->undo_items[0]));
        undo_manager->undo_items[UNDO_MANAGER_MAX_UNDOS - 1] = *item;
    } else {
        undo_manager->undo_items[undo_manager->index++] = *item;
    }
    undo_manager->undo_marker = undo_manager->undo_marker + 1;
}


int undo_manager_can_undo(const UndoManager *undo_manager) {
    return undo_manager->index > 0;
}


int undo_manager_undo(UndoManager *undo_manager) {
    if (!undo_manager_can_undo(undo_manager)) {
        return FALSE;
    }
    UndoItem undo_item = undo_manager->undo_items[--undo_manager->index];
    undo_item.undo_function(undo_item.object);
    undo_manager->undo_marker = undo_manager->undo_marker - 1;
    return TRUE;
}


int undo_manager_all_undone(const UndoManager *undo_manager) {
    return undo_manager->undo_marker == 0;
}


void undo_manager_reset_undo_marker(UndoManager *undo_manager) {
    undo_manager->undo_marker = 0;
}


void undo_manager_reset(UndoManager *undo_manager) {
    undo_manager->index = 0;
    undo_manager->undo_marker = 0;
}
