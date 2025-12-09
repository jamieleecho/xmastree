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


WNDSCR mywindow = {
    "xmastree",     /* window title */
    0,              /* number of menus */
    40,             /* min. window width */
    24,             /* min. window height */
    0xC0C0,       	/* sync bytes */
    {0,0,0,0,0,0,0},/* reserved */
    NULL           	/* pointer to menu descriptors */
};


int main(int argc, char **argv) {
    int local_sig;
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
        printf("Got signal %d, %d\n", msinfo.pt_acx, msinfo.pt_acy);
    }

    return 0;
}
