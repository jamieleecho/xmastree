#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "app.h"


#define KEY_SIG  11  /* signal number for key interrupts */

#define MOUSE_UPDATE_PERIOD    3  /* check every 3 interrupts */
#define MOUSE_TIMEOUT_PERIOD  10  /* timeout every 10 interrupts */
#define MOUSE_FOLLOW           1  /* update gc immediately */
#define MOUSE_SIG             10  /* signal number for mouse interrupts */

#define FOREGROUND_COLOR 0
#define BACKGROUND_COLOR 15

#define DIALOG_WIDTH  26
#define DIALOG_HEIGHT 10

#define BUTTON_WIDTH  8
#define BUTTON_HEIGHT 1
#define FONT_WIDTH    8
#define FONT_HEIGHT   8

#define PATH_TEXTBOX_WIDTH (DIALOG_WIDTH - 4)
#define PATH_TEXTBOX_HEIGHT 2


typedef enum {
    UiObjectType_Button,
    UiObjectType_TextBox,
} UiObjectType;

typedef struct {
    const char *text;
    int accelerator_key;
} UiButtonOptions;

typedef struct {
    char *text;
} UiTextBoxOptions;

typedef union {
    UiButtonOptions button;
    UiTextBoxOptions text_box;
} UiObjectOptions;

typedef struct {
    UiObjectType object_type;
    int x;
    int y;
    int width;
    int height;
    UiObjectOptions options;
} UiObject;


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
        do {
            _cgfx_ss_ssig(OUTPATH, KEY_SIG);
            _cgfx_ss_mssig(OUTPATH, MOUSE_SIG);
            sleep();
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
    SCF_OPT options;
    _cgfx_gs_opt(path, &options);
    options.sg_echo = on;
    _cgfx_ss_opt(path, &options);
}


void run_application(WNDSCR *mywindow, const MenuItemAction *menu_actions,
                     void (*application_action)(UiEvent *event)) {
    int local_sig, itemno, menuid, ii;
    MenuItemAction const * menu_item_action;
    UiEvent event;

    echo_sw(OUTPATH, 0);
    intercept();

    _cgfx_curoff(OUTPATH);
    _cgfx_tcharsw(OUTPATH, FALSE);
    _cgfx_scalesw(OUTPATH, FALSE);
    _cgfx_setgc(OUTPATH, GRP_PTR, PTR_ARR);
    _cgfx_ss_mouse(OUTPATH, MOUSE_UPDATE_PERIOD, MOUSE_TIMEOUT_PERIOD, MOUSE_FOLLOW);

    int err = _cgfx_ss_wnset(0, WT_FWIN, mywindow);

    while(TRUE) {
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


static void draw_button(const UiObject *object) {
    _cgfx_curxy(OUTPATH, object->x, object->y);
    Flush();
    write(OUTPATH, object->options.button.text,
         strlen(object->options.button.text));
}


static void set_normal_video(void) {
    _cgfx_fcolor(OUTPATH, FOREGROUND_COLOR);
    _cgfx_bcolor(OUTPATH, BACKGROUND_COLOR);
}


static void set_reverse_video(const UiObject *object) {
    _cgfx_bcolor(OUTPATH, FOREGROUND_COLOR);
    _cgfx_fcolor(OUTPATH, BACKGROUND_COLOR);
}


#if 0
static void set_standard_text_mode(void) {
    _cgfx_fcolor(OUTPATH, FOREGROUND_COLOR);
    _cgfx_bcolor(OUTPATH, BACKGROUND_COLOR);
    _cgfx_boldsw(OUTPATH, FALSE);
    _cgfx_tcharsw(OUTPATH, FALSE);
    _cgfx_curoff(OUTPATH);
}
#endif


static void focus_text_box(const UiObject *object) {
    set_reverse_video(object);
    _cgfx_boldsw(OUTPATH, TRUE);
    _cgfx_curon(OUTPATH);
}


static void unfocus_text_box(const UiObject *object) {
    set_normal_video();
    _cgfx_curoff(OUTPATH);
}


static void draw_text_box(const UiObject *object) {
    int ii, sz, delta;
    _cgfx_fcolor(OUTPATH, FOREGROUND_COLOR);
    _cgfx_setdptr(OUTPATH, object->x * FONT_WIDTH, object->y * FONT_HEIGHT);
    _cgfx_rbar(OUTPATH, object->width * FONT_WIDTH, object->height * FONT_HEIGHT);
    focus_text_box(object);
    _cgfx_curxy(OUTPATH, object->x, object->y);
    Flush();
    for(ii = 0, sz = strlen(object->options.text_box.text); sz > 0;) {
        delta = write(OUTPATH, object->options.text_box.text + ii,
                      min(sz, PATH_TEXTBOX_WIDTH - 1));
        if (delta <= 0) {
            break;
        }
        sz = sz - delta;
        ii = ii + delta;
        if (sz) {
            _cgfx_curdwn(OUTPATH);
            Flush();
        }
    }
    unfocus_text_box(object);
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
    set_normal_video();
}


int handle_button_key_event(const UiObject *object, char c) {
    return (c == object->options.button.accelerator_key);
}


int handle_text_box_key_event(UiObject *object, char c) {
    if (((c >= 0x20) || (c == '\b')) && !(c & 0x80)) {
        const char saved_c = c;
        int sz = strlen(object->options.text_box.text);
        int x, y;
        if (c == '\b') {
            if (sz == 0) {
                return TRUE;
            }
            sz = sz - 1;
            object->options.text_box.text[sz] = 0;
            c = ' ';
        } else {
            if (sz >= APP_PATH_MAX - 1) {
                _cgfx_bell(OUTPATH);
                Flush();
                return TRUE;
            }

            object->options.text_box.text[sz] = c;
            object->options.text_box.text[sz + 1] = 0;
        }

        x = sz % PATH_TEXTBOX_WIDTH;
        y = sz / PATH_TEXTBOX_WIDTH;
        _cgfx_curxy(OUTPATH, x + object->x, y + object->y);
        Flush();
        write(OUTPATH, &c, 1);
        if (saved_c == '\b') {
            _cgfx_curxy(OUTPATH, x + object->x, y + object->y);
            Flush();
        }
        return TRUE;
    }
    return FALSE;
}


static int handle_key_event(UiObject *object, char c) {
    switch(object->object_type) {
        case UiObjectType_Button:
            return handle_button_key_event(object, c);
        case UiObjectType_TextBox:
            return handle_text_box_key_event(object, c);
    }
    return 0;
}


static int wait_for_button_press(UiObject *objects, int num_objects, UiObject **key_object) {
    UiEvent event;
    UiObject *obj;
    int ii;

    if (key_object && *key_object && (*key_object)->object_type == UiObjectType_TextBox) {
        focus_text_box(*key_object);
    }

    while (TRUE) {
        run_event_loop(&event);

        if (event.event_type == UiEventType_KeyPress) {
            /* Send key press to designated key object first */
            if (key_object && *key_object) {
                obj = *key_object;
                if (handle_key_event(obj, event.info.key.character)) {
                    /* The event was handled, but if it is a button we may need to return */
                    if (obj->object_type == UiObjectType_Button) {
                        /* Find the index of the object so we can indicate the button was pressed */
                        for(ii=0; ii<num_objects; ii++) {
                            if (objects + ii == obj) {
                                return ii;
                            }
                        }
                    }

                    /* Resume looking for events because the event did not dismiss dialog */
                    continue;
                }
            }

            /* The key object did not handle the event, forward key press to other objects */
            for (ii = 0; ii < num_objects; ++ii) {
                obj = objects + ii;
                if (key_object && (obj == *key_object)) {
                    continue;
                }
                if (obj->object_type == UiObjectType_Button) {
                    if (handle_key_event(obj, event.info.key.character)) {
                        return ii;
                    }
                }
            }
        }

       if (event.event_type != UiEventType_MouseClick) {
            continue;
        }

        if (key_object && *key_object && (*key_object)->object_type == UiObjectType_TextBox) {
            set_normal_video();
        }

        if (event.info.mouse.pt_valid && event.info.mouse.pt_cbsa) {
            event.info.mouse.pt_wrx = event.info.mouse.pt_wrx / FONT_WIDTH;
            event.info.mouse.pt_wry = event.info.mouse.pt_wry / FONT_HEIGHT;
            for (ii = 0; ii < num_objects; ++ii) {
                obj = objects + ii;
                if (event.info.mouse.pt_wrx >= obj->x &&
                    event.info.mouse.pt_wrx < obj->x + obj->width &&
                    event.info.mouse.pt_wry >= obj->y &&
                    event.info.mouse.pt_wry < obj->y + obj->height) {
                    return ii;
                }
            }
        }
    }

    return -1;
}


static MessageBoxResult show_generic_message_box(
    const char *message, char *path, MessageBoxType event_type) {
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
    if (event_type >= MessageBoxType_OkCancel) {
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

    switch(event_type) {
        case MessageBoxType_OkCancel:
        case MessageBoxType_SaveAs:
        case MessageBoxType_Open:
            objects[0].options.button.text = "[  OK  ]";
            objects[0].options.button.accelerator_key = '\r';
            objects[1].options.button.text = "[Cancel]";
            objects[1].options.button.accelerator_key = -1;
            break;
        case MessageBoxType_YesNo:
            num_objects = 2;
            objects[0].options.button.text = "[ Yes ]";
            objects[0].options.button.accelerator_key = '\r';
            objects[1].options.button.text = "[ No  ]";
            objects[1].options.button.accelerator_key = -1;
            break;
        default:
            num_objects = 1;
            objects[0].options.button.text = "[  OK  ]";
            objects[0].options.button.accelerator_key = '\r';
            break;
    }

    if (event_type == MessageBoxType_Open || event_type == MessageBoxType_SaveAs) {
        num_objects = 3;
        objects[2].object_type = UiObjectType_TextBox;
        objects[2].x = (DIALOG_WIDTH - PATH_TEXTBOX_WIDTH - 2) / 2;
        objects[2].y = DIALOG_HEIGHT - BUTTON_HEIGHT - PATH_TEXTBOX_HEIGHT - 4;
        objects[2].width = PATH_TEXTBOX_WIDTH;
        objects[2].height = PATH_TEXTBOX_HEIGHT;
        objects[2].options.text_box.text = path;
    }

    draw_objects(objects, num_objects);

    UiObject *key_object = (num_objects == 3) ? objects + 2 : (UiObject *)NULL;
    MessageBoxResult result = (MessageBoxResult)wait_for_button_press(&objects, num_objects, &key_object);
    _cgfx_owend(OUTPATH);
    return result;
}


MessageBoxResult show_message_box(const char *message, MessageBoxType event_type) {
    return show_generic_message_box(message, NULL, event_type);
}


static char path_buffer[APP_PATH_MAX];

char *show_open_dialog(char *path) {
    path[APP_PATH_MAX - 1] = 0;
    strcpy(path_buffer, path);
    MessageBoxResult result = show_generic_message_box(
        "Open File:", path_buffer, MessageBoxType_Open);
    if (result == MessageBoxResult_Ok) {
        strcpy(path, path_buffer);
        return path;
    }
    return (char *)NULL;
}


char *show_save_dialog(char *path) {
    path[APP_PATH_MAX - 1] = 0;
    strcpy(path_buffer, path);
    MessageBoxResult result = show_generic_message_box(
        "Save File As:", path_buffer, MessageBoxType_SaveAs);
    if (result == MessageBoxResult_Ok) {
        strcpy(path, path_buffer);
        return path;
    }
    return (char *)NULL;
}
