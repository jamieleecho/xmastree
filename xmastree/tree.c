#include <stdio.h>
#include "tree.h"

void tree_controller_init(tree_controller *controller) {
}


void tree_controller_new(tree_controller *controller) {
}

void tree_controller_open(tree_controller *controller) {
}


void tree_controller_save(tree_controller *controller) {
}


void tree_controller_save_as(tree_controller *controller) {
}


void tree_controller_add_item(tree_controller *controller, tree_item_t item) {
}


void tree_controller_undo(tree_controller *controller) {
}


void tree_controller_redo(tree_controller *controller) {
}


BOOL tree_controller_can_undo(const tree_controller *controller) {
    return FALSE;
}


BOOL tree_controller_can_redo(const tree_controller *controller) {
    return FALSE;
}


BOOL tree_controller_has_filename(const tree_controller *controller) {
    return FALSE;
}


void tree_new(tree_t *tree) {
}


void tree_load(tree_t *tree, const char *filename) {
}


void tree_save(const tree_t *tree, const char *filename) {
}


void tree_add_item(tree_t *tree, tree_item_t item) {
}


void tree_remove_last_item(tree_t *tree) {
}


int tree_get_item_count(const tree_t *tree) {
}
