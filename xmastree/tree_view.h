#ifndef _TREE_VIEW_H
#define _TREE_VIEW_H

#include "app.h"
#include "toolbox.h"
#include "tree.h"

#define TREE_VIEW_X 10
#define TREE_VIEW_Y 0
#define TREE_VIEW_WIDTH (320 - TREE_VIEW_X - (TOOLBOX_ITEM_HEIGHT * TOOLBOX_NUM_ITEMS))
#define TREE_VIEW_HEIGHT (200 - 16 - TREE_VIEW_Y)


typedef struct {
    int x;
    int y;
    int width;
    int height;
    Tree *tree;
    int item_id;
    int image_ids[TOOLBOX_NUM_ITEMS];
} TreeView;

extern void tree_view_init(TreeView *view, Tree *tree, int item_id, const int *image_ids);
extern int tree_view_handle_event(TreeView *view, UiEvent *event);
extern void tree_view_refresh(const TreeView *view);
extern void tree_view_set_item_id(TreeView *view, int item_id);

#endif /* _TREE_VIEW_H */