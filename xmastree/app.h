#ifndef _APP_H_
#define _APP_H_

#include <stdbool.h>
#include <stdlib.h>
#include <cgfx.h>

#define INPATH 0
#define OUTPATH 1
#define APP_PATH_MAX 44  /* char array for path length */

#define MN_HELP 30


typedef enum {
    UiEventType_KeyPress,
    UiEventType_MouseClick
} UiEventType;

typedef struct {
    char character;
} KeyEvent;

typedef struct {
    UiEventType event_type;
    union {
        KeyEvent key;
        MSRET mouse;
    } info;
} UiEvent;


typedef struct {
    int menuid;
    int itemno;
    void (*action)(MSRET *msinfo, int menuid, int itemno);
} MenuItemAction;


typedef enum {
    MessageBoxType_Info,
    MessageBoxType_Warning,
    MessageBoxType_Error,
    MessageBoxType_OkCancel,
    MessageBoxType_YesNo,
    MessageBoxType_Open,
    MessageBoxType_SaveAs
} MessageBoxType;


typedef enum {
    MessageBoxResult_Ok = 0,
    MessageBoxResult_Cancel,
    MessageBoxResult_Yes = 0,
    MessageBoxResult_No
} MessageBoxResult;

extern void app_init(const int *palette, size_t num_colors);
extern void run_application(
    WNDSCR *mywindow,
    void (*init)(void),
    const MenuItemAction *menu_actions,
    void (*refresh_menus_action)(void),
    void (*application_action)(UiEvent *event)
);
extern void app_refresh_menubar();
extern MessageBoxResult show_message_box(const char *message, MessageBoxType type);
extern char *show_open_dialog(char *path);
extern char *show_save_dialog(char *path);

#endif /* _APP_H_ */
