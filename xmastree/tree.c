#include <stdio.h>
#include "tree.h"

void tree_controller_init(XmasTreeController *controller) {
}


void tree_controller_new(XmasTreeController *controller) {
}

void tree_controller_open(XmasTreeController *controller) {
}


void tree_controller_save(XmasTreeController *controller) {
}


void tree_controller_save_as(XmasTreeController *controller) {
}


void tree_controller_add_item(XmasTreeController *controller, XmasTreeItem item) {
}


void tree_controller_undo(XmasTreeController *controller) {
}


void tree_controller_redo(XmasTreeController *controller) {
}


BOOL tree_controller_can_add(const XmasTreeController *controller) {
    return FALSE;
}


BOOL tree_controller_can_undo(const XmasTreeController *controller) {
    return FALSE;
}


BOOL tree_controller_can_redo(const XmasTreeController *controller) {
    return FALSE;
}


BOOL tree_controller_has_filename(const XmasTreeController *controller) {
    return FALSE;
}


void tree_new(XmasTree *tree) {
}


void tree_load(XmasTree *tree, const char *filename) {
}


void tree_save(const XmasTree *tree, const char *filename) {
}


void tree_add_item(XmasTree *tree, XmasTreeItem item) {
}


void tree_remove_last_item(XmasTree *tree) {
}


void tree_add_last_item_back(XmasTree *tree) {
}


int tree_get_item_count(const XmasTree *tree) {
    return 0;
}
