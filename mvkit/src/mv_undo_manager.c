#include <string.h>

#include "mvkit/mv_undo_manager.h"


void mv_undo_manager_init(MVUndoManager *undo_manager) {
    mv_undo_manager_reset(undo_manager);
}


void mv_undo_manager_push_undo(MVUndoManager *undo_manager, const MVUndoItem *item) {
    if (undo_manager->index >= (MV_UNDO_MANAGER_MAX_UNDOS - 1)) {
        memcpy((char *)undo_manager->undo_items, (char *)(undo_manager->undo_items + 1), sizeof(undo_manager->undo_items) - sizeof(undo_manager->undo_items[0]));
        undo_manager->undo_items[MV_UNDO_MANAGER_MAX_UNDOS - 1] = *item;
    } else {
        undo_manager->undo_items[undo_manager->index++] = *item;
    }
    undo_manager->undo_marker = undo_manager->undo_marker + 1;
}


bool mv_undo_manager_can_undo(const MVUndoManager *undo_manager) {
    return undo_manager->index > 0;
}


bool mv_undo_manager_undo(MVUndoManager *undo_manager) {
    if (!mv_undo_manager_can_undo(undo_manager)) {
        return false;
    }
    MVUndoItem undo_item = undo_manager->undo_items[--undo_manager->index];
    undo_item.undo_function(undo_item.object);
    undo_manager->undo_marker = undo_manager->undo_marker - 1;
    return true;
}


bool mv_undo_manager_all_undone(const MVUndoManager *undo_manager) {
    return undo_manager->undo_marker == 0;
}


void mv_undo_manager_reset_undo_marker(MVUndoManager *undo_manager) {
    undo_manager->undo_marker = 0;
}


void mv_undo_manager_reset(MVUndoManager *undo_manager) {
    undo_manager->index = 0;
    undo_manager->undo_marker = 0;
}
