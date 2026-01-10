#include <stdio.h>
#include <string.h>

#include <cgfx.h>

#include "app.h"
#include "image.h"
#include "toolbox.h"


#define TOOLBOX_BACKGROUND_COLOR 0
#define TOOLBOX_FOREGROUND_COLOR 1


void tool_box_init(ToolBox *toolbox, int x, int y, const int *image_ids, void (*item_selected)(struct ToolBox *toolbox)) {
    toolbox->x = x;
    toolbox->y = y;
    toolbox->width = TOOLBOX_WIDTH;
    toolbox->height = TOOLBOX_HEIGHT;
    toolbox->is_visible = true;
    memcpy(toolbox->image_ids, image_ids, sizeof(toolbox->image_ids));
    toolbox->item = 0;
    toolbox->item_selected = item_selected;
    tool_box_refresh(toolbox);
}


bool tool_box_select_item_at_xy(ToolBox *toolbox, int x, int y) {
    if (!toolbox->is_visible ||
        (x < toolbox->x) || (x >= toolbox->x + toolbox->width) ||
        (y < toolbox->y) || (y >= toolbox->y + toolbox->height)) {
        return false;
    }
    x = (x - toolbox->x) / TOOLBOX_BUTTON_WIDTH;
    y = (y - toolbox->y) / TOOLBOX_BUTTON_HEIGHT;
    return tool_box_select_item(toolbox, y * TOOLBOX_ITEMS_PER_ROW + x);
}


static void tool_box_toggle_item_selection(ToolBox *toolbox) {
    int item = toolbox->item;
    int button_x = toolbox->x + (item % TOOLBOX_ITEMS_PER_ROW) * TOOLBOX_BUTTON_WIDTH + 1;
    int button_y = toolbox->y + (item / TOOLBOX_ITEMS_PER_ROW) * TOOLBOX_BUTTON_HEIGHT + 1;
    _cgfx_lset(OUTPATH, LOG_XOR);
    _cgfx_fcolor(OUTPATH, TOOLBOX_FOREGROUND_COLOR);
    _cgfx_setdptr(OUTPATH, button_x, button_y);
    _cgfx_rbar(OUTPATH, TOOLBOX_BUTTON_WIDTH - 2, TOOLBOX_BUTTON_HEIGHT - 2);
    Flush();
}


bool tool_box_select_item(ToolBox *toolbox, int item) {
    if ((item < 0) || (item > TOOLBOX_NUM_ITEMS)) {
        return false;
    }

    if (item == toolbox->item) {
        return true;
    }

    tool_box_toggle_item_selection(toolbox);
    toolbox->item = item;
    tool_box_toggle_item_selection(toolbox);
    if (toolbox->item_selected) {
        toolbox->item_selected(toolbox);
    }
    return true;
}


int tool_box_item(ToolBox *toolbox) {
    return toolbox->item;
}


void tool_box_clear(ToolBox *toolbox) {
    _cgfx_lset(OUTPATH, LOG_NONE);
    _cgfx_fcolor(OUTPATH, TOOLBOX_BACKGROUND_COLOR);
    _cgfx_setdptr(OUTPATH, toolbox->x, toolbox->y);
    _cgfx_rbar(OUTPATH, toolbox->width, toolbox->height);
}


void tool_box_set_is_visible(ToolBox *toolbox, bool is_visible) {
    toolbox->is_visible = is_visible;
    tool_box_refresh(toolbox);
    Flush();
}


bool tool_box_is_visible(ToolBox *toolbox) {
    return toolbox->is_visible;
}


void tool_box_refresh(ToolBox *toolbox){
    tool_box_clear(toolbox);
    if (!toolbox->is_visible) {
        return;
    }
    _cgfx_fcolor(OUTPATH, TOOLBOX_FOREGROUND_COLOR);
    for(int yy = 0; yy < TOOLBOX_NUM_ITEMS; yy += 1) {
        for(int xx = 0; xx < TOOLBOX_ITEMS_PER_ROW; xx = xx + 1) {
            int item = (yy * TOOLBOX_ITEMS_PER_ROW) + xx;
            if (item >= TOOLBOX_NUM_ITEMS) {
                break;
            }
            int x = (xx * TOOLBOX_BUTTON_WIDTH) + toolbox->x;
            int y = (yy * TOOLBOX_BUTTON_HEIGHT) + toolbox->y;
            _cgfx_setdptr(OUTPATH, x, y);
            _cgfx_rbox(OUTPATH, TOOLBOX_BUTTON_WIDTH, TOOLBOX_BUTTON_HEIGHT);
            image_draw_image(toolbox->image_ids[item], x + 1, y + 1);
        }
    }

    tool_box_toggle_item_selection(toolbox);
}
