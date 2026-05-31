/*
 * The smallest runnable MVKit app: a framed window titled "minimal" with no
 * menu bar (mv_menu_none) and no content, driven entirely by the default no-op
 * handlers. It exists to exercise the Phase 4 app-init surface end to end and
 * to runtime-verify a zero-menu window.
 *
 * The one concession to usability is a close handler: a no-menu app otherwise
 * has no way to quit from within Multi-Vue (the window's close box raises an
 * MN_CLOS selection, which the no-op dispatch table would just ignore).
 */

#include <unistd.h>   /* exit */

#include <mvkit/mvkit.h>


static void minimal_close(MSRET *msinfo, int menuid, int itemno) {
    (void)msinfo;
    (void)menuid;
    (void)itemno;
    exit(0);
}

static void minimal_unhandled(MSRET *msinfo, int menuid, int itemno) {
    (void)msinfo;
    (void)menuid;
    (void)itemno;
}

static const MVMenuItemAction minimal_actions[] = {
    { MN_CLOS, 1, minimal_close },   /* window close box */
    { -1, -1, minimal_unhandled },   /* catch-all sentinel */
};


mv_menu_none(minimal_window, "minimal");


int main(int argc, char **argv) {
    return mv_app_run(argc, argv, &minimal_window,
        mv_app_pre_init_nop, mv_app_init_nop, minimal_actions,
        mv_app_refresh_menus_action_nop, mv_app_event_nop);
}
