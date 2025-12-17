#ifndef _TOOLBOX_H
#define _TOOLBOX_H


#define TOOLBOX_NUM_ITEMS 10

typedef struct ToolBox {
    int reserved;
    int x;
    int y;
    int width;
    int height;
    int is_visible;
    int image_ids[10];
    int item;
    void (*item_selected)(struct ToolBox *toolbox);
} ToolBox;


extern void tool_box_init(ToolBox *toolbox, int x, int y, const int *image_ids, void (*item_selected)(struct ToolBox *toolbox));
extern int tool_box_select_item_at_xy(ToolBox *toolbox, int x, int y);
extern int tool_box_select_item(ToolBox *toolbox, int item);
extern int tool_box_item(ToolBox *toolbox);
extern void tool_box_clear(ToolBox *toolbox);
extern void tool_box_set_is_visible(ToolBox *toolbox, int is_visible);
extern int tool_box_is_visible(ToolBox *toolbox);
extern void tool_box_refresh(ToolBox *toolbox);

#endif