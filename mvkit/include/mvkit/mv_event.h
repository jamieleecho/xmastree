#ifndef _MVKIT_MV_EVENT_H
#define _MVKIT_MV_EVENT_H

#include <cgfx.h>   /* MSRET */

/*
 * @file
 * Input events delivered to views and the application. Kept in its own
 * foundational header so lower-level pieces (e.g. mv_view) can use events
 * without depending on the application module.
 */

/** Which kind of input an MVUiEvent carries. */
typedef enum {
    MVUiEventType_KeyPress,
    MVUiEventType_MouseClick
} MVUiEventType;

/** Payload of a key-press event: the character read from the keyboard. */
typedef struct {
    char character;
} MVKeyEvent;

/** A UI event delivered to a view or the application action callback.
   `event_type` selects the active member of `info`: a MVKeyEvent for key
   presses, or the cgfx MSRET mouse record for clicks. */
typedef struct {
    MVUiEventType event_type;
    union {
        MVKeyEvent key;
        MSRET mouse;
    } info;
} MVUiEvent;

#endif /* _MVKIT_MV_EVENT_H */
