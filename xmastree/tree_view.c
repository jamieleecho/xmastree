#include <cgfx.h>
#include <stdio.h>
#include <string.h>

#include "image.h"

#include "tree_view.h"


#define TREE_VIEW_BACKGROUND_COLOR 0


void tree_view_init(TreeView *view, Tree *tree, int item_id, const int *image_ids) {
    view->x = TREE_VIEW_X;
    view->y = TREE_VIEW_Y;
    view->width = TREE_VIEW_WIDTH;
    view->height = TREE_VIEW_HEIGHT;
    view->tree = tree;
    view->item_id = item_id;
    memcpy(view->image_ids, image_ids, sizeof(view->image_ids));
}


static void tree_view_draw_item(int x, int y, int image_id) {
    _cgfx_lset(OUTPATH, LOG_AND);
    image_draw_image(image_id - 1, x, y);
    _cgfx_lset(OUTPATH, LOG_XOR);
    image_draw_image(image_id, x, y);
    _cgfx_lset(OUTPATH, LOG_NONE);
}


int tree_view_handle_event(TreeView *view, UiEvent *event) {
    if (event->event_type != UiEventType_MouseClick) {
        return FALSE;
    }

    int x = event->info.mouse.pt_wrx;
    int y = event->info.mouse.pt_wry;
    x = x - TOOLBOX_ITEM_WIDTH / 2;
    y = y - TOOLBOX_ITEM_HEIGHT / 2;
    TreeItem item = {x + view->x, y + view->y, view->item_id};
    tree_add_item(view->tree, &item);
    tree_view_draw_item(x, y, view->image_ids[view->item_id]);
    Flush();
    return TRUE;
}


void tree_view_refresh(const TreeView *view) {
    printf("here!!!!\r\n");
    _cgfx_lset(OUTPATH, LOG_NONE);
    _cgfx_bcolor(OUTPATH, TREE_VIEW_BACKGROUND_COLOR);
    _cgfx_setdptr(OUTPATH, view->x, view->y);
    _cgfx_rbar(OUTPATH, view->width, view->height);

    for(int ii = 0; ii < view->tree->num_items; ++ii) {
        const TreeItem *item = view->tree->items + ii;
        _cgfx_setdptr(OUTPATH, view->x, view->y);
        tree_view_draw_item(item->x, item->y, view->image_ids[item->item_id]);
    }
    Flush();
}


void tree_view_set_item_id(TreeView *view, int item_id) {
    view->item_id = item_id;
}
