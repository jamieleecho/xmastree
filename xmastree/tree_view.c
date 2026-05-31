#include <cgfx.h>

#include <mvkit/mv_image.h>

#include "tree_view.h"


#define TREE_VIEW_BACKGROUND_COLOR 0

/* Ornament images are 24x24; clicks are centered on the cursor. */
#define TREE_VIEW_ORNAMENT_WIDTH  24
#define TREE_VIEW_ORNAMENT_HEIGHT 24


static void tree_view_draw_item(int x, int y, int image_id) {
    _cgfx_lset(MV_OUTPATH, LOG_AND);
    mv_image_draw(image_id - 1, x, y);
    _cgfx_lset(MV_OUTPATH, LOG_XOR);
    mv_image_draw(image_id, x, y);
    _cgfx_lset(MV_OUTPATH, LOG_NONE);
}


static void tree_view_draw(MVView *v) {
    TreeView *view = (TreeView *)v;

    _cgfx_lset(MV_OUTPATH, LOG_NONE);
    _cgfx_fcolor(MV_OUTPATH, TREE_VIEW_BACKGROUND_COLOR);
    _cgfx_setdptr(MV_OUTPATH, v->x, v->y);
    _cgfx_rbar(MV_OUTPATH, v->width - 1, v->height - 1);
    Flush();

    for (int ii = 0; ii < view->tree->num_items; ++ii) {
        const TreeItem *item = view->tree->items + ii;
        tree_view_draw_item(item->x + v->x, item->y + v->y, view->image_ids[item->item_id]);
    }
    Flush();
}


static bool tree_view_handle_click(MVView *v, MVUiEvent *event) {
    TreeView *view = (TreeView *)v;
    int x, y;

    if (event->event_type != MVUiEventType_MouseClick) {
        return false;
    }
    x = event->info.mouse.pt_wrx - TREE_VIEW_ORNAMENT_WIDTH / 2;
    y = event->info.mouse.pt_wry - TREE_VIEW_ORNAMENT_HEIGHT / 2;
    if ((x < v->x) || (y < v->y)) {
        return false;
    }
    TreeItem item = {x - v->x, y - v->y, view->item_id};
    tree_add_item(view->tree, &item);
    tree_view_draw_item(x, y, view->image_ids[view->item_id]);
    Flush();
    return true;
}


void tree_view_init(TreeView *view, int x, int y, int width, int height,
                    Tree *tree, int item_id, const int *image_ids) {
    view->view.x = x;
    view->view.y = y;
    view->view.width = width;
    view->view.height = height;
    view->view.is_visible = true;
    view->view.draw = tree_view_draw;
    view->view.handle_click = tree_view_handle_click;
    view->tree = tree;
    view->item_id = item_id;
    view->image_ids = image_ids;
}


void tree_view_refresh(TreeView *view) {
    tree_view_draw(&view->view);
}


void tree_view_set_item_id(TreeView *view, int item_id) {
    view->item_id = item_id;
}
