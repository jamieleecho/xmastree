#include <cgfx.h>


#define INPATH 0
#define OUTPATH 1


typedef struct {
    int menuid;
    int itemno;
    void (*action)(MSRET *msinfo, int menuid, int itemno);
} menu_item_action_t;


extern void run_application(WNDSCR *mywindow, const menu_item_action_t *menu_actions);
