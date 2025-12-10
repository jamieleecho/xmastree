#include <stdio.h>
#include <cgfx.h>

#define NULL ((void *)0)
#define WT_FWIN 2

extern void Flush(void);

/*** xmastree defines ***/

#define MOUSE_UPDATE_PERIOD    3  /* check every 3 interrupts */
#define MOUSE_TIMEOUT_PERIOD  10  /* timeout every 10 interrupts */
#define MOUSE_FOLLOW           1  /* update gc immediately */
#define MOUSE_SIG             10  /* signal number for mouse interrupts */

#define INPATH 0
#define OUTPATH 1


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


MIDSCR file_menu_items[] = {
    {"Exit", MN_ENBL, {0, 0, 0, 0, 0}}
};

MNDSCR menus[] = {
    {
        "File",         /* menu title */
        MN_FILE,        /* menu id */
        10,             /* menu width */
        1,              /* number of items */
        MN_ENBL,        /* menu enabled */
        {0, 0},         /* reserved */
        file_menu_items /* pointer to items */
    }
};

WNDSCR mywindow = {
    "xmastree",             /* window title */
    1,                      /* number of menus */
    40,                     /* min. window width */
    24,                     /* min. window height */
    0xC0C0,       	        /* sync bytes */
    {0, 0, 0, 0, 0, 0, 0},  /* reserved */
    menus                   /* pointer to menu descriptors */
};


int main(int argc, char **argv) {
    int local_sig, itemno, menuid;
    MSRET msinfo;

    intercept();

    _cgfx_setgc(OUTPATH, GRP_PTR, PTR_ARR);
    _cgfx_ss_mouse(OUTPATH, MOUSE_UPDATE_PERIOD, MOUSE_TIMEOUT_PERIOD, MOUSE_FOLLOW);

    printf("Got signal\n");
    int err = _cgfx_ss_wnset(0, WT_FWIN, &mywindow);

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
            menuid = 99;
            _cgfx_gs_mnsel(OUTPATH, &itemno, &menuid);
            printf("Menu %d Item %d selected\n", menuid, itemno);
            switch (menuid) {
                case MN_CLOS:
                case MN_FILE:
                    if (itemno == 0) {
                        return 0;
                    }
                    break;
            }
        } else if (msinfo.pt_stat == WR_CNTNT) {
        }
    }

    return 0;
}
