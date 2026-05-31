#ifndef _MVKIT_MV_IMAGE_GRID_H
#define _MVKIT_MV_IMAGE_GRID_H

#include <mvkit/mv_view.h>

/* Default item geometry and colors used by mv_image_grid_init(). */
#define MV_IMAGE_GRID_ITEM_WIDTH  24
#define MV_IMAGE_GRID_ITEM_HEIGHT 24
#define MV_IMAGE_GRID_ITEM_BORDER 1
/* Foreground = lightest chrome register (white), background = darkest (black),
   per the reg 0-3 chrome ramp -- maximum contrast for the button borders. */
#define MV_IMAGE_GRID_DEFAULT_FG  3
#define MV_IMAGE_GRID_DEFAULT_BG  0

/*
 * MVImageGrid is a grid of single-select image buttons -- the generalized form
 * of xmastree's tool palette. It conforms to MVView (embedded first), so an app
 * dispatches clicks to it via mv_view_dispatch_click() and draws it via
 * mv_view_draw(). The selected item is highlighted with an XOR rectangle.
 */
typedef struct MVImageGrid {
    MVView view;          /**< base; frame is computed from the layout in init */
    int num_items;        /**< number of buttons */
    int columns;          /**< buttons per row */
    int item_width;       /**< image width inside each button */
    int item_height;      /**< image height inside each button */
    int fg_color;         /**< button border / highlight color */
    int bg_color;         /**< background fill color */
    const int *image_ids; /**< num_items image-buffer ids (borrowed) */
    int selected;         /**< index of the currently selected item */
    void (*item_selected)(struct MVImageGrid *self);  /**< called on selection change (may be NULL) */
} MVImageGrid;

/** Initialize a grid at (x, y) with `num_items` buttons laid out `columns` wide,
   drawing `image_ids[0..num_items)`. `item_selected` (may be NULL) is called
   when the selection changes. Item size defaults to 24x24 and colors to fg 1 /
   bg 0 (set the fields afterward to override colors); the view frame is computed
   from the layout. Draws the grid immediately. */
extern void mv_image_grid_init(MVImageGrid *grid, int x, int y,
                               int num_items, int columns,
                               const int *image_ids,
                               void (*item_selected)(MVImageGrid *self));

/** Index of the currently selected item. */
extern int mv_image_grid_selected(const MVImageGrid *grid);

/** Select item `item` (0-based). No-op if already selected; returns false if
   `item` is out of range. Updates the highlight and fires item_selected. */
extern bool mv_image_grid_select(MVImageGrid *grid, int item);

/** Show or hide the grid (and redraw). */
extern void mv_image_grid_set_visible(MVImageGrid *grid, bool is_visible);

#endif /* _MVKIT_MV_IMAGE_GRID_H */
