#ifndef _TOOLBOX_H
#define _TOOLBOX_H


#define TOOLBOX_NUM_ITEMS 10
#define TOOLBOX_ITEM_WIDTH 24
#define TOOLBOX_ITEM_HEIGHT 24

#define TOOLBOX_ITEM_BORDER_WIDTH 1
#define TOOLBOX_ITEM_BORDER_HEIGHT 1
#define TOOLBOX_BUTTON_WIDTH (TOOLBOX_ITEM_WIDTH + TOOLBOX_ITEM_BORDER_WIDTH)
#define TOOLBOX_BUTTON_HEIGHT (TOOLBOX_ITEM_HEIGHT + TOOLBOX_ITEM_BORDER_HEIGHT)

#define TOOLBOX_ITEMS_PER_ROW 2
#define TOOLBOX_WIDTH (TOOLBOX_BUTTON_WIDTH * TOOLBOX_ITEMS_PER_ROW)
#define TOOLBOX_HEIGHT (TOOLBOX_BUTTON_HEIGHT * ((TOOLBOX_NUM_ITEMS + TOOLBOX_ITEMS_PER_ROW - 1)/ TOOLBOX_ITEMS_PER_ROW))


typedef struct ToolBox {
    int reserved;
    int x;
    int y;
    int width;
    int height;
    bool is_visible;
    int image_ids[10];
    int item;
    void (*item_selected)(struct ToolBox *toolbox);
} ToolBox;


extern void tool_box_init(ToolBox *toolbox, int x, int y, const int *image_ids, void (*item_selected)(struct ToolBox *toolbox));
extern bool tool_box_select_item_at_xy(ToolBox *toolbox, int x, int y);
extern bool tool_box_select_item(ToolBox *toolbox, int item);
extern int tool_box_item(ToolBox *toolbox);
extern void tool_box_clear(ToolBox *toolbox);
extern void tool_box_set_is_visible(ToolBox *toolbox, bool is_visible);
extern bool tool_box_is_visible(ToolBox *toolbox);
extern void tool_box_refresh(ToolBox *toolbox);

#endif