#ifndef _APP_H_
#define _APP_H_

#include <cgfx.h>

#define INPATH 0
#define OUTPATH 1
#define PATH_MAX 43  /* max path length for files */

#define MN_HELP 30


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


extern void run_application(WNDSCR *mywindow, const MenuItemAction *menu_actions);
extern MessageBoxResult show_message_box(const char *message, MessageBoxType type, int default_button);
char *show_open_dialog(char *path);
char *show_save_dialog(char *path);

#endif /* _APP_H_ */
