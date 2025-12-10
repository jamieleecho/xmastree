#include <stdio.h>
#include "app.h"


MIDSCR file_menu_items[] = {
    {"New", MN_ENBL, {0, 0, 0, 0, 0}},
    {"----------", MN_DSBL, {0, 0, 0, 0, 0}},
    {"Open...", MN_ENBL, {0, 0, 0, 0, 0}},
    {"Save", MN_ENBL, {0, 0, 0, 0, 0}},
    {"Save As...", MN_ENBL, {0, 0, 0, 0, 0}},
    {"----------", MN_DSBL, {0, 0, 0, 0, 0}},
    {"Exit", MN_ENBL, {0, 0, 0, 0, 0}},
};

MNDSCR menus[] = {
    {
        "Files",         /* menu title */
        MN_FILE,         /* menu id */
        11,              /* menu width */
        7,               /* number of items */
        MN_ENBL,         /* menu enabled */
        {0, 0},          /* reserved */
        file_menu_items  /* pointer to items */
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


void exit_action(MSRET *msinfo, int menuid, int itemno) {
    printf("Exiting application...\n");
}


void new_action(MSRET *msinfo, int menuid, int itemno) {
    printf("New file action selected.\n");
}


void open_action(MSRET *msinfo, int menuid, int itemno) {
    printf("Open file action selected.\n");
}


void save_action(MSRET *msinfo, int menuid, int itemno) {
    printf("Save file action selected.\n");
}


void save_as_action(MSRET *msinfo, int menuid, int itemno) {
    printf("Save As action selected.\n");
}


void unknown_action(MSRET *msinfo, int menuid, int itemno) {
    printf("Menu ID: %d, Item No: %d\n", menuid, itemno);
}

menu_item_action_t menu_actions[] = {
    {MN_CLOS, 1, exit_action},
    {MN_FILE, 1, new_action},
    {MN_FILE, 3, open_action},
    {MN_FILE, 4, save_action},
    {MN_FILE, 5, save_as_action},
    {MN_FILE, 7, exit_action},
    {-1, -1, unknown_action}
};


int main(int argc, char **argv) {
    run_application(&mywindow, menu_actions);

    return 0;
}
