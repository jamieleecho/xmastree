/*
 * The smallest runnable MVKit app: a framed window titled "minimal" with no
 * menu bar (mv_menu_none) and no content, driven entirely by the default no-op
 * handlers. It exercises the Phase 4 app-init surface end to end and
 * runtime-verifies a zero-menu Multi-Vue window.
 *
 * There is no explicit quit code: mv_app_run quits on the window close box by
 * default, so even an all-no-op app can be dismissed.
 */

#include <mvkit/mvkit.h>


mv_menu_none(minimal_window, "minimal");


int main(int argc, char **argv) {
    return mv_app_run(argc, argv, &minimal_window,
        mv_app_pre_init_nop, mv_app_init_nop, mv_app_menu_actions_nop,
        mv_app_refresh_menus_action_nop, mv_app_event_nop);
}
