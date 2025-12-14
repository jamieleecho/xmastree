#include <stdio.h>
#include <unistd.h>
#include "app.h"
#include "document.h"
#include "version.h"

static Document xmastree_doc;

static MIDSCR file_menu_items[] = {
    {"New", MN_ENBL, {0, 0, 0, 0, 0}},
    {"----------", MN_DSBL, {0, 0, 0, 0, 0}},
    {"Open...", MN_ENBL, {0, 0, 0, 0, 0}},
    {"Save", MN_ENBL, {0, 0, 0, 0, 0}},
    {"Save As...", MN_ENBL, {0, 0, 0, 0, 0}},
    {"----------", MN_DSBL, {0, 0, 0, 0, 0}},
    {"Exit", MN_ENBL, {0, 0, 0, 0, 0}},
};

static MIDSCR help_menu_items[] = {
    {"About...", MN_ENBL, {0, 0, 0, 0, 0}},
};

static MNDSCR menus[] = {
    {
        "File",          /* menu title */
        MN_FILE,         /* menu id */
        11,              /* menu width */
        7,               /* number of items */
        MN_ENBL,         /* menu enabled */
        {0, 0},          /* reserved */
        file_menu_items  /* pointer to items */
    },
    {
        "Help",          /* menu title */
        MN_HELP,         /* menu id */
        11,              /* menu width */
        1,               /* number of items */
        MN_ENBL,         /* menu enabled */
        {0, 0},          /* reserved */
        help_menu_items  /* pointer to items */
    }
};

static WNDSCR mywindow = {
    "xmastree",                     /* window title */
    sizeof(menus)/sizeof(MNDSCR),   /* number of menus */
    40,                             /* min. window width */
    24,                             /* min. window height */
    0xC0C0,       	                /* sync bytes */
    {0, 0, 0, 0, 0, 0, 0},          /* reserved */
    menus                           /* pointer to menu descriptors */
};


static void exit_action(MSRET *msinfo, int menuid, int itemno) {
    if (document_is_dirty(&xmastree_doc)) {
        if (document_save(&xmastree_doc)) {
            exit(0);
        }
    }
}


static void new_action(MSRET *msinfo, int menuid, int itemno) {
    document_new(&xmastree_doc);
}


static void open_action(MSRET *msinfo, int menuid, int itemno) {
    document_open(&xmastree_doc);
}


static void save_action(MSRET *msinfo, int menuid, int itemno) {
    document_save(&xmastree_doc);
}


static void save_as_action(MSRET *msinfo, int menuid, int itemno) {
    document_save_as(&xmastree_doc);
}


static void unknown_action(MSRET *msinfo, int menuid, int itemno) {
    printf("Menu ID: %d, Item No: %d\n", menuid, itemno);
}


static void about_action(MSRET *msinfo, int menuid, int itemno) {
    show_message_box("     xmastree v" APP_VERSION "\r\n    Build xmas trees!", MessageBoxType_Info);
}


static MenuItemAction menu_actions[] = {
    {MN_CLOS, 1, exit_action},
    {MN_FILE, 1, new_action},
    {MN_FILE, 3, open_action},
    {MN_FILE, 4, save_action},
    {MN_FILE, 5, save_as_action},
    {MN_FILE, 7, exit_action},
    {MN_HELP, 1, about_action},
    {-1, -1, unknown_action}
};


int new_model(void *model, const char *path) {
    return 0;
}


int open_model(void *model, const char *path) {
    return 0;
}


int save_model(void *model, const char *path) {
    return 0;
}


int main(int argc, char **argv) {
    document_init(
        &xmastree_doc,
        NULL,
        "xmastree.xmt",
        NULL,
        new_model,
        open_model,
        save_model
    );

    run_application(&mywindow, menu_actions);

    return 0;
}
