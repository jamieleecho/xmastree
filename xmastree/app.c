#include <fcntl.h>
#include <os.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "app.h"
#include "file_dialog.h"


#define KEY_SIG  11  /* signal number for key interrupts */

#define MOUSE_UPDATE_PERIOD    3  /* check every 3 interrupts */
#define MOUSE_TIMEOUT_PERIOD  10  /* timeout every 10 interrupts */
#define MOUSE_FOLLOW           1  /* update gc immediately */
#define MOUSE_SIG             10  /* signal number for mouse interrupts */

#define FOREGROUND_COLOR 0
#define BACKGROUND_COLOR 3

void app_init(const int *palette, size_t num_colors) {
    for (size_t ii = 0; ii < num_colors; ++ii) {
        _cgfx_palette(OUTPATH, ii, palette[ii]);
    }
}


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


static void run_event_loop(UiEvent *event) {
    int local_sig;
    bool first = true;

    while(true) {
        sigcode = 0;
        do {
            _cgfx_ss_mssig(OUTPATH, MOUSE_SIG);
            do {
                sigcode = 0;
                _cgfx_ss_mssig(OUTPATH, MOUSE_SIG);
            }
            while (first && sigcode);
            first = false;
            _ss_ssig(OUTPATH, KEY_SIG);
            sleep(1);
        } while (sigcode == 0);
        local_sig = sigcode;

        if (local_sig == KEY_SIG) {
            char c;
            if (!read(INPATH, &c, 1)) {
                c = 0;
            }
            event->event_type = UiEventType_KeyPress;
            event->info.key.character = c;
            return;
        } else if (local_sig == MOUSE_SIG) {
            event->event_type = UiEventType_MouseClick;
            _cgfx_gs_mouse(OUTPATH, &event->info.mouse);
            return;
        }
    }
}


void echo_sw(path_id path, char on) {
    struct sgbuf options;
    _gs_opt(path, &options);
    options.sg_echo = on;
    _ss_opt(path, &options);
}


static int refresh_menu_bar;


extern void app_refresh_menubar() {
    refresh_menu_bar = true;
}


void run_application(WNDSCR *mywindow, void (*init)(void),
                     const MenuItemAction *menu_actions,
                     void (*refresh_menus_action)(void),
                     void (*application_action)(UiEvent *event)) {
    int local_sig, itemno, menuid, ii;
    MenuItemAction const * menu_item_action;
    UiEvent event;

    echo_sw(OUTPATH, 0);
    intercept();

    _cgfx_curoff(OUTPATH);
    _cgfx_tcharsw(OUTPATH, false);
    _cgfx_scalesw(OUTPATH, false);
    _cgfx_setgc(OUTPATH, GRP_PTR, PTR_ARR);
    _cgfx_ss_mouse(OUTPATH, MOUSE_UPDATE_PERIOD, MOUSE_TIMEOUT_PERIOD, MOUSE_FOLLOW);

    int err = _cgfx_ss_wnset(0, WT_FWIN, mywindow);
    if (init) {
        init();
    }
    refresh_menu_bar = true;

    while(true) {
        if (refresh_menu_bar) {
            if (refresh_menus_action) {
                refresh_menus_action();
            }

            _cgfx_ss_umbar(OUTPATH);
            refresh_menu_bar = false;
        }

        run_event_loop(&event);

        if (event.event_type == UiEventType_KeyPress) {
            if (application_action) {
                application_action(&event);
            }
            continue;
        }

        /* must be a mouse event */

        if (event.info.mouse.pt_valid == 0) {
            continue;
        }

        if (event.info.mouse.pt_stat == WR_CNTRL) {
            /* handle menus */
            _cgfx_gs_mnsel(OUTPATH, &itemno, &menuid);
            for (ii=0; menu_actions[ii].menuid >= 0; ++ii) {
                menu_item_action = menu_actions + ii;
                if (menu_item_action->menuid == menuid &&
                    menu_item_action->itemno == itemno) {
                    menu_item_action->action(&event.info.mouse, menuid, itemno);
                    break;
                }
            }

            /* unhandled menu */
            if (menu_actions[ii].menuid < 0) {
                menu_actions[ii].action(&event.info.mouse, menuid, itemno);
            }

            continue;
        }

        /* handle content window events */
        if (event.info.mouse.pt_stat == WR_CNTNT) {
            if (application_action) {
                application_action(&event);
            }
        }
    }
}


static int app_dialog_centered(DIALOG *dialog, int width, int height) {
    int sx, sy, result;

    if (_cgfx_gs_scsz(OUTPATH, &sx, &sy)) {
        sx = 0;
        sy = 0;
    } else {
        sx = (sx - width) / 2;
        sy = (sy - height) / 2;
        if (sx < 0) {
            sx = 0;
        }
        if (sy < 0) {
            sy = 0;
        }
    }

    result = Dialog(OUTPATH, dialog, sx, sy, width, height,
                    FOREGROUND_COLOR, BACKGROUND_COLOR);
    return result;
}


/* Message box drawn with cmoc_os9's button widget via Dialog(). The frame
   costs one character on each edge, so usable columns are 1..(WIDTH-2). */
#define MSGBOX_WIDTH      28
#define MSGBOX_HEIGHT     10
#define MSGBOX_INTERIOR   (MSGBOX_WIDTH - 2)
#define MSGBOX_TEXT_ROW   2
#define MSGBOX_BUTTON_ROW 6
#define MSGBOX_MAX_LINES  3

#define MSGBOX_VAL_PRIMARY   1  /* OK / Yes */
#define MSGBOX_VAL_SECONDARY 2  /* Cancel / No */

static int add_button(DIALOG *item, int column, int row, int key, int val, char *label) {
    item->d_type = D_BUTTON;
    item->d_column = (char)column;
    item->d_row = (char)row;
    item->d_key = (char)key;
    item->d_val = (char)val;
    item->d_string = label;
    return 1;
}

MessageBoxResult show_message_box(const char *message, MessageBoxType event_type) {
    char buf[80];
    char *lines[MSGBOX_MAX_LINES];
    DIALOG items[MSGBOX_MAX_LINES + 3];  /* lines + up to 2 buttons + D_END */
    int num_lines = 0;
    int n = 0;
    int val;
    char *p;

    /* Split the message into up to MSGBOX_MAX_LINES lines on CR/LF. */
    strncpy(buf, message, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    p = buf;
    lines[num_lines++] = p;
    while (*p && num_lines < MSGBOX_MAX_LINES) {
        if (*p == '\r' || *p == '\n') {
            *p++ = 0;
            while (*p == '\r' || *p == '\n') {
                p++;
            }
            if (!*p) {
                break;
            }
            lines[num_lines++] = p;
        } else {
            p++;
        }
    }

    /* Text lines, centered horizontally within the interior. */
    for (int ii = 0; ii < num_lines; ++ii) {
        int len = strlen(lines[ii]);
        items[n].d_type = D_TEXT;
        items[n].d_column = (char)(1 + (MSGBOX_INTERIOR - len) / 2);
        items[n].d_row = (char)(MSGBOX_TEXT_ROW + ii);
        items[n].d_key = 0;
        items[n].d_val = 0;
        items[n].d_string = lines[ii];
        n++;
    }

    /* Buttons, centered as a row, depending on the box type. */
    if (event_type == MessageBoxType_YesNo) {
        n += add_button(&items[n],  8, MSGBOX_BUTTON_ROW, '\r', MSGBOX_VAL_PRIMARY,   (char *)" Yes ");
        n += add_button(&items[n], 15, MSGBOX_BUTTON_ROW, 0x1b, MSGBOX_VAL_SECONDARY, (char *)" No ");
    } else if (event_type == MessageBoxType_OkCancel) {
        n += add_button(&items[n],  8, MSGBOX_BUTTON_ROW, '\r', MSGBOX_VAL_PRIMARY,   (char *)" OK ");
        n += add_button(&items[n], 14, MSGBOX_BUTTON_ROW, 0x1b, MSGBOX_VAL_SECONDARY, (char *)"Cancel");
    } else {
        n += add_button(&items[n], 11, MSGBOX_BUTTON_ROW, '\r', MSGBOX_VAL_PRIMARY,   (char *)"  OK  ");
    }

    items[n].d_type = D_END;
    items[n].d_column = 0;
    items[n].d_row = 0;
    items[n].d_key = 0;
    items[n].d_val = 0;
    items[n].d_string = (char *)NULL;

    val = app_dialog_centered(items, MSGBOX_WIDTH, MSGBOX_HEIGHT);

    switch (event_type) {
        case MessageBoxType_YesNo:
            return (val == MSGBOX_VAL_PRIMARY) ? MessageBoxResult_Yes : MessageBoxResult_No;
        case MessageBoxType_OkCancel:
            return (val == MSGBOX_VAL_PRIMARY) ? MessageBoxResult_Ok : MessageBoxResult_Cancel;
        default:
            return MessageBoxResult_Ok;
    }
}


/* app_file_dialog()'s box footprint (incl. WT_DBOX border), for centering. */
#define FILEDIALOG_WIDTH  24
#define FILEDIALOG_HEIGHT 14

/* Center app_file_dialog() on screen and copy the chosen name into `path`.
   Returns `path` on confirm, NULL on cancel. The browser starts in the CWD;
   `path` is an output buffer only. `allow_new` adds the "[new file]" entry.
   `ext` is the document extension (NULL for none); a leading dot is stripped
   since the file browser wants it dotless (e.g. "xmt", not ".xmt").
   On confirm the CWD is left at the chosen file's directory; on cancel the
   dialog restores the starting CWD. */
static char *app_file_dialog_centered(const char *title, const char *confirm_label,
                                      int allow_new, const char *ext, char *path) {
    int sx, sy;
    char *result;

    if (ext && *ext == '.') {
        ext++;
    }

    if (_cgfx_gs_scsz(OUTPATH, &sx, &sy)) {
        sx = 0;
        sy = 0;
    } else {
        sx = (sx - FILEDIALOG_WIDTH) / 2;
        sy = (sy - FILEDIALOG_HEIGHT) / 2;
        if (sx < 0) {
            sx = 0;
        }
        if (sy < 0) {
            sy = 0;
        }
    }

    result = app_file_dialog(OUTPATH, title, confirm_label, allow_new, ext,
                             sx, sy, FOREGROUND_COLOR, BACKGROUND_COLOR);

    if (result) {
        strncpy(path, result, APP_PATH_MAX - 1);
        path[APP_PATH_MAX - 1] = 0;
        return path;
    }
    return (char *)NULL;
}

char *show_open_dialog(char *path, const char *ext) {
    return app_file_dialog_centered("Open File", "Open", 0, ext, path);
}


char *show_save_dialog(char *path, const char *ext) {
    return app_file_dialog_centered("Save File", "Save", 1, ext, path);
}
