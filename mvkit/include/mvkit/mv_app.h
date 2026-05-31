#ifndef _MVKIT_MV_APP_H
#define _MVKIT_MV_APP_H

#include <stdlib.h>   /* size_t */
#include <cgfx.h>     /* WNDSCR, MSRET */
#include <stdbool.h>
#include <mvkit/mv_defs.h>

typedef enum {
    MVUiEventType_KeyPress,
    MVUiEventType_MouseClick
} MVUiEventType;

typedef struct {
    char character;
} MVKeyEvent;

typedef struct {
    MVUiEventType event_type;
    union {
        MVKeyEvent key;
        MSRET mouse;
    } info;
} MVUiEvent;

typedef struct {
    int menuid;
    int itemno;
    void (*action)(MSRET *msinfo, int menuid, int itemno);
} MVMenuItemAction;

typedef enum {
    MVMessageBoxType_Info,
    MVMessageBoxType_Warning,
    MVMessageBoxType_Error,
    MVMessageBoxType_OkCancel,
    MVMessageBoxType_YesNo
} MVMessageBoxType;

typedef enum {
    MVMessageBoxResult_Ok = 0,
    MVMessageBoxResult_Cancel,
    MVMessageBoxResult_Yes = 0,
    MVMessageBoxResult_No
} MVMessageBoxResult;

extern void mv_app_init(const int *palette, size_t num_colors);
extern void mv_app_run(
    WNDSCR *mywindow,
    void (*init)(void),
    const MVMenuItemAction *menu_actions,
    void (*refresh_menus_action)(void),
    void (*application_action)(MVUiEvent *event)
);
extern void mv_app_refresh_menubar(void);
extern MVMessageBoxResult mv_app_show_message_box(const char *message, MVMessageBoxType type);
extern char *mv_app_show_open_dialog(char *path, const char *ext);
extern char *mv_app_show_save_dialog(char *path, const char *ext);

#endif /* _MVKIT_MV_APP_H */
