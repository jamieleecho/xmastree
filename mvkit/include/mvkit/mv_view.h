#ifndef _MVKIT_MV_VIEW_H
#define _MVKIT_MV_VIEW_H

#include <stdbool.h>
#include <mvkit/mv_app.h>   /* MVUiEvent */

/*
 * MVView is MVKit's minimal view protocol -- loosely NSView's core: a frame, a
 * draw entry point, and click handling. Concrete views (e.g. MVImageGrid, or an
 * app's own content view) embed an MVView as their FIRST member, so a
 * `ConcreteView *` can be used as an `MVView *`. There is no view hierarchy or
 * automatic event routing yet; an app keeps its own list of views and dispatches
 * to them (see mv_view_dispatch_click).
 */
typedef struct MVView {
    int x;             /**< left edge, screen coords */
    int y;             /**< top edge, screen coords */
    int width;         /**< frame width */
    int height;        /**< frame height */
    bool is_visible;   /**< when false, the view neither draws nor handles clicks */
    void (*draw)(struct MVView *self);                       /**< redraw the view */
    bool (*handle_click)(struct MVView *self, MVUiEvent *e);  /**< hit-test + handle; true if consumed */
} MVView;

/** True if (x, y) lies within the view's frame. A convenience for view
   implementations that want a simple rectangular hit-test. */
extern bool mv_view_contains_point(const MVView *view, int x, int y);

/** Draw the view if it is visible and has a draw function. */
extern void mv_view_draw(MVView *view);

/** Offer a click to the view: if it is visible and has a handler, call it and
   return whether it consumed the event; otherwise return false. The handler is
   responsible for its own hit-testing (views may want non-rectangular bounds). */
extern bool mv_view_dispatch_click(MVView *view, MVUiEvent *event);

#endif /* _MVKIT_MV_VIEW_H */
