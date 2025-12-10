#ifndef _TREE_H_
#define _TREE_H_

#include <os.h>

#define TREE_MAX_ITEMS 100

typedef struct {
    int x;
    int y;
    int item_type;
} tree_item_t;


typedef struct {
    int num_items;
    tree_item_t items[TREE_MAX_ITEMS];
} tree_t;


typedef struct {
    tree_t model;
    int min_index;
    int max_index;
    char filename[256];
} tree_controller;


extern void tree_controller_init(tree_controller *controller);
extern void tree_controller_new(tree_controller *controller);
extern void tree_controller_open(tree_controller *controller);
extern void tree_controller_save(tree_controller *controller);
extern void tree_controller_save_as(tree_controller *controller);
extern void tree_controller_add_item(tree_controller *controller, tree_item_t item);
extern void tree_controller_undo(tree_controller *controller);
extern void tree_controller_redo(tree_controller *controller);
extern BOOL tree_controller_can_undo(const tree_controller *controller);
extern BOOL tree_controller_can_redo(const tree_controller *controller);
extern BOOL tree_controller_has_filename(const tree_controller *controller);

extern void tree_new(tree_t *tree);
extern void tree_load(tree_t *tree, const char *filename);
extern void tree_save(const tree_t *tree, const char *filename);
extern void tree_add_item(tree_t *tree, tree_item_t item);
extern void tree_remove_last_item(tree_t *tree);
int tree_get_item_count(const tree_t *tree);

#endif /* _TREE_H_ */