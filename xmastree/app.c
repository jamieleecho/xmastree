#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "app.h"

extern void Flush(void);

#define KEY_SIG  11  /* signal number for key interrupts */

#define MOUSE_UPDATE_PERIOD    3  /* check every 3 interrupts */
#define MOUSE_TIMEOUT_PERIOD  10  /* timeout every 10 interrupts */
#define MOUSE_FOLLOW           1  /* update gc immediately */
#define MOUSE_SIG             10  /* signal number for mouse interrupts */

#define FOREGROUND_COLOR 1
#define BACKGROUND_COLOR 0

#define DIALOG_WIDTH  26
#define DIALOG_HEIGHT 10

#define BUTTON_WIDTH  8
#define BUTTON_HEIGHT 1
#define FONT_WIDTH    8
#define FONT_HEIGHT   8

#define PATH_TEXTBOX_WIDTH (DIALOG_WIDTH - 4)
#define PATH_TEXTBOX_HEIGHT 2


typedef enum {
    UiEvent_Keypress,
    UiEvent_MouseClick
} UiEventType;


typedef struct {
    int __unused__;
} KeyEvent;


typedef struct {
    UiEventType type;
    union {
        KeyEvent key;
        MSRET mouse;
    } info;
} UiEvent;


typedef enum {
    UiObjectType_Button,
    UiObjectType_TextBox,
} UiObjectType;

typedef struct {
    UiObjectType object_type;
    int x;
    int y;
    int width;
    int height;
    const char *text;
} UiObject;


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


static void run_event_loop(UiEvent *event) {
    int local_sig;

    while(TRUE) {
        sigcode = 0;
        while(sigcode == 0) {
            _cgfx_ss_ssig(OUTPATH, KEY_SIG);
            _cgfx_ss_mssig(OUTPATH, MOUSE_SIG);
            sleep();
        }
        local_sig = sigcode;
        sigcode = 0;

        if (local_sig == KEY_SIG) {
            event->type = UiEvent_Keypress;
            return;
        } else if (local_sig == MOUSE_SIG) {
            event->type = UiEvent_MouseClick;
            _cgfx_gs_mouse(OUTPATH, &event->info.mouse);
            return;
        }
    }
}


void echo_sw(path_id path, int on) {
    SCF_OPT options;
    _cgfx_gs_opt(path, &options);
    options.sg_echo = on;
    _cgfx_ss_opt(path, &options);
}


void run_application(WNDSCR *mywindow, const MenuItemAction *menu_actions) {
    int local_sig, itemno, menuid, ii;
    MenuItemAction const * menu_item_action;
    UiEvent event;

    echo_sw(OUTPATH, 0);
    intercept();

    _cgfx_setgc(OUTPATH, GRP_PTR, PTR_ARR);
    _cgfx_ss_mouse(OUTPATH, MOUSE_UPDATE_PERIOD, MOUSE_TIMEOUT_PERIOD, MOUSE_FOLLOW);

    int err = _cgfx_ss_wnset(0, WT_FWIN, mywindow);

    while(TRUE) {
        run_event_loop(&event);

        if (event.type == UiEvent_Keypress) {
            char ch;
            read(OUTPATH, &ch, 1);
            printf("Key pressed: %c (0x%02X)\n", (ch >= 32 && ch <= 126) ? ch : '.', (unsigned char)ch);
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
        }
    }
}


static void draw_button(const UiObject *object) {
    _cgfx_curxy(OUTPATH, object->x, object->y);
    Flush();
    write(OUTPATH, object->text, strlen(object->text));
}


static void unfocus_text_box(void) {
    _cgfx_fcolor(OUTPATH, FOREGROUND_COLOR);
    _cgfx_bcolor(OUTPATH, BACKGROUND_COLOR);
    _cgfx_cwarea(OUTPATH, 1, 1, DIALOG_WIDTH - 2, DIALOG_HEIGHT - 2);
    Flush();
}


static void draw_text_box(const UiObject *object) {
    _cgfx_cwarea(OUTPATH, object->x, object->y, object->width, object->height);
    _cgfx_bcolor(OUTPATH, FOREGROUND_COLOR);
    _cgfx_fcolor(OUTPATH, BACKGROUND_COLOR);
    Flush();
    write(OUTPATH, "\f", 1);
    write(OUTPATH, object->text, strlen(object->text));
    unfocus_text_box();
}


static void draw_objects(const UiObject *objects, int num_objects) {
    for (int ii = 0; ii < num_objects; ++ii) {
        const UiObject *obj = objects + ii;
        switch(obj->object_type) {
            case UiObjectType_Button:
                draw_button(obj);
                break;
            case UiObjectType_TextBox:
                draw_text_box(obj);
                break;
        }
    }
}


static int wait_for_button_press(const UiObject *objects, int num_objects) {
    UiEvent event;

    while (TRUE) {
        run_event_loop(&event);

        if (event.type != UiEvent_MouseClick) {
            continue;
        }

        if (event.info.mouse.pt_valid && event.info.mouse.pt_cbsa) {
            event.info.mouse.pt_wrx = event.info.mouse.pt_wrx / FONT_WIDTH;
            event.info.mouse.pt_wry = event.info.mouse.pt_wry / FONT_HEIGHT;
            for (int ii = 0; ii < num_objects; ++ii) {
                if (event.info.mouse.pt_wrx >= objects[ii].x &&
                    event.info.mouse.pt_wrx < objects[ii].x + objects[ii].width &&
                    event.info.mouse.pt_wry >= objects[ii].y &&
                    event.info.mouse.pt_wry < objects[ii].y + objects[ii].height) {
                    return ii;
                }
            }
        }
    }

    return -1;
}


static MessageBoxResult show_generic_message_box(
    const char *message, char *path, MessageBoxType type,
    int default_button) {
    UiObject objects[3];
    int sx, sy;
    int num_objects;

    if (_cgfx_gs_scsz(OUTPATH, &sx, &sy) || sx < DIALOG_WIDTH || sy < DIALOG_HEIGHT) {
        printf("Error: Screen too small for dialog box.\n");
        exit(1);
    }

    sx = (sx - DIALOG_WIDTH) / 2;
    sy = (sy - DIALOG_HEIGHT) / 2;
    _cgfx_owset(OUTPATH, 1, sx, sy, DIALOG_WIDTH, DIALOG_HEIGHT, FOREGROUND_COLOR, BACKGROUND_COLOR);
    _cgfx_curoff(OUTPATH);
    _cgfx_boldsw(OUTPATH, 1);
    _cgfx_ss_wnset(OUTPATH, WT_DBOX, NULL);
    Flush();
    write(OUTPATH, message, strlen(message));

    sy = DIALOG_HEIGHT - BUTTON_HEIGHT - 1 - 2;
    if (type >= MessageBoxType_OkCancel) {
        num_objects = 2;
        sx = (DIALOG_WIDTH - 4) / 2 - BUTTON_WIDTH;
    } else {
        num_objects = 1;
        sx = (DIALOG_WIDTH - BUTTON_WIDTH - 2) / 2;
    }

    for (int ii = 0; ii < num_objects; ++ii) {
        objects[ii].object_type = UiObjectType_Button;
        objects[ii].x = sx + ii * (BUTTON_WIDTH + 2);
        objects[ii].y = sy;
        objects[ii].width = BUTTON_WIDTH;
        objects[ii].height = BUTTON_HEIGHT;
    }

    switch(type) {
        case MessageBoxType_OkCancel:
        case MessageBoxType_SaveAs:
        case MessageBoxType_Open:
            objects[0].text = "[  OK  ]";
            objects[1].text = "[Cancel]";
            break;
        case MessageBoxType_YesNo:
            num_objects = 2;
            objects[0].text = "[ Yes ]";
            objects[1].text = "[ No  ]";
            break;
        default:
            num_objects = 1;
            objects[0].text = "[  OK  ]";
            break;
    }

    if (type == MessageBoxType_Open || type == MessageBoxType_SaveAs) {
        num_objects = 3;
        objects[2].object_type = UiObjectType_TextBox;
        objects[2].x = (DIALOG_WIDTH - PATH_TEXTBOX_WIDTH) / 2;
        objects[2].y = DIALOG_HEIGHT - BUTTON_HEIGHT - PATH_TEXTBOX_HEIGHT - 4;
        objects[2].width = PATH_TEXTBOX_WIDTH;
        objects[2].height = PATH_TEXTBOX_HEIGHT;
        objects[2].text = path;
    }

    draw_objects(objects, num_objects);

    MessageBoxResult result = (MessageBoxResult)wait_for_button_press(&objects, num_objects);
    _cgfx_owend(OUTPATH);
    return result;
}


MessageBoxResult show_message_box(const char *message,
    MessageBoxType type, int default_button) {
    return show_generic_message_box(message, NULL, type, default_button);
}


char *show_open_dialog(char *path) {
    MessageBoxResult result = show_generic_message_box(
        "Open File:\r\n\r\n", path, MessageBoxType_Open, 0);
    return (result == MessageBoxResult_Ok) ? path : (char *)NULL;
}


char *show_save_dialog(char *path) {
    MessageBoxResult result = show_generic_message_box(
        "Save File As:\r\n\r\n", path, MessageBoxType_SaveAs, 0);
    return (result == MessageBoxResult_Ok) ? path : (char *)NULL;
}
