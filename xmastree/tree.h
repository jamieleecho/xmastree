#ifndef _TREE_H_
#define _TREE_H_

#include <os.h>

#define TREE_MAX_ITEMS 100

typedef struct {
    int x;
    int y;
    int item_type;
} XmasTreeItem;


typedef struct {
    int num_items;
    XmasTreeItem items[TREE_MAX_ITEMS];
} XmasTree;


typedef struct {
    XmasTree model;
    int min_index;
    int max_index;
    char filename[256];
} XmasTreeController;


extern void tree_controller_init(XmasTreeController *controller);
extern void tree_controller_new(XmasTreeController *controller);
extern void tree_controller_open(XmasTreeController *controller);
extern void tree_controller_save(XmasTreeController *controller);
extern void tree_controller_save_as(XmasTreeController *controller);
extern void tree_controller_add_item(XmasTreeController *controller, XmasTreeItem item);
extern void tree_controller_undo(XmasTreeController *controller);
extern void tree_controller_redo(XmasTreeController *controller);
extern BOOL tree_controller_can_add(const XmasTreeController *controller);
extern BOOL tree_controller_can_undo(const XmasTreeController *controller);
extern BOOL tree_controller_can_redo(const XmasTreeController *controller);
extern BOOL tree_controller_has_filename(const XmasTreeController *controller);

extern void tree_new(XmasTree *tree);
extern void tree_load(XmasTree *tree, const char *filename);
extern void tree_save(const XmasTree *tree, const char *filename);
extern void tree_add_item(XmasTree *tree, XmasTreeItem item);
extern void tree_remove_last_item(XmasTree *tree);
extern void tree_add_last_item_back(XmasTree *tree);
int tree_get_item_count(const XmasTree *tree);

#endif /* _TREE_H_ */