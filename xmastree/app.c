#include <stdio.h>
#include "app.h"

#define NULL ((void *)0)
extern void Flush(void);


#define MOUSE_UPDATE_PERIOD    3  /* check every 3 interrupts */
#define MOUSE_TIMEOUT_PERIOD  10  /* timeout every 10 interrupts */
#define MOUSE_FOLLOW           1  /* update gc immediately */
#define MOUSE_SIG             10  /* signal number for mouse interrupts */


static char sigcode = 0;
asm void sighandler(void) {
    asm {
        stb ,u
        rti
    }
}


void
intercept()
{
    asm
    {
        pshs    u
        leax    sighandler
        leau    sigcode
        os9     F$Icpt
        puls    u
    }
}


asm void
sleep(void)
{
    asm
    {
        os9     F$Sleep
        rts
    }
}


void run_application(WNDSCR *mywindow, const menu_item_action_t *menu_actions) {
    int local_sig, itemno, menuid, ii;
    menu_item_action_t const * menu_item_action;
    MSRET msinfo;

    intercept();

    _cgfx_setgc(OUTPATH, GRP_PTR, PTR_ARR);
    _cgfx_ss_mouse(OUTPATH, MOUSE_UPDATE_PERIOD, MOUSE_TIMEOUT_PERIOD, MOUSE_FOLLOW);

    int err = _cgfx_ss_wnset(0, WT_FWIN, mywindow);

    while(TRUE) {
        _cgfx_ss_mssig(OUTPATH, MOUSE_SIG);
        while(sigcode == 0) {
            sleep();
        }
        local_sig = sigcode;
        sigcode = 0;

        _cgfx_gs_mouse(OUTPATH, &msinfo);
        if (msinfo.pt_valid == 0) {
            continue;
        } else if (msinfo.pt_stat == WR_CNTRL) {
            _cgfx_gs_mnsel(OUTPATH, &itemno, &menuid);

            for (ii=0; menu_actions[ii].menuid >= 0; ++ii) {
                menu_item_action = menu_actions + ii;
                if (menu_item_action->menuid == menuid &&
                    menu_item_action->itemno == itemno) {
                    menu_item_action->action(&msinfo, menuid, itemno);
                    break;
                }
            }
            if (menu_actions[ii].menuid < 0) {
                menu_actions[ii].action(&msinfo, menuid, itemno);
            }
        } else if (msinfo.pt_stat == WR_CNTNT) {
        }
    }
}