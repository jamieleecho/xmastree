#include <stdio.h>
#include <stdlib.h>
#include <cgfx.h>

#define NULL ((void *)0)
#define WT_FWIN 2


static int sigcode;
void sighandler(int sig) {
    sigcode = sig;
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
    intercept(sighandler);
    int err = _cgfx_ss_wnset(0, WT_FWIN, &mywindow);

    return 0;
}
