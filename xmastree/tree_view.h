#ifndef _TREE_VIEW_H
#define _TREE_VIEW_H

#include <mvkit/mv_view.h>
#include "tree.h"


/* xmastree's content view: an MVView that renders the tree model and turns
   clicks into "add an ornament" edits. Embeds MVView first so it can be
   dispatched to via mv_view_dispatch_click() / mv_view_draw(). */
typedef struct {
    MVView view;
    Tree *tree;
    int item_id;
    const int *image_ids;
} TreeView;

extern void tree_view_init(TreeView *view, int x, int y, int width, int height,
                           Tree *tree, int item_id, const int *image_ids);
extern void tree_view_refresh(TreeView *view);
extern void tree_view_set_item_id(TreeView *view, int item_id);

#endif /* _TREE_VIEW_H */
