#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <mvkit/mvkit.h>

#include "tree.h"
#include "tree_view.h"
#include "version.h"


/* xmastree's theme. Registers 0-3 are the OS-9 window-chrome ramp: darkest ->
   lightest (black, dark grey, light grey, white), which cowin hardcodes for the
   menu bar, dropdowns, dialog borders, shadows and scrollbars. Registers 4-15
   are the tree/ornament colors. The MVTheme union initializes raw[]; the named
   ramp.* fields alias it. */
static const MVTheme theme = { {
    0x00, 0x07, 0x38, 0x3f, 0x04, 0x05, 0x06, 0x10,
    0x36, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x01, 0x02
} };

#define XMAS_BACKGROUND 0

#define MN_HELP 30  /* app-chosen Help menu id (not a cgfx constant) */

/* Layout: a 2-column tool palette at top-left, the tree canvas to its right. */
#define XMAS_NUM_ORNAMENTS 10
#define TOOLBOX_COLUMNS    2
#define TOOLBOX_X          4
#define TOOLBOX_Y          4
#define TOOLBOX_WIDTH      (TOOLBOX_COLUMNS * (MV_IMAGE_GRID_ITEM_WIDTH + MV_IMAGE_GRID_ITEM_BORDER))
#define TREE_VIEW_X        (10 + TOOLBOX_WIDTH)
#define TREE_VIEW_Y        0
#define TREE_VIEW_WIDTH    (320 - 16 - TREE_VIEW_X)
#define TREE_VIEW_HEIGHT   (200 - 16 - TREE_VIEW_Y)

static Tree tree;
static MVDocument xmastree_doc;


typedef enum {
    FileMenuIndex_Save = 3,
} FileMenuIndex;

static MIDSCR file_menu_items[] = {
    MV_MENU_ITEM("New"),
    MV_MENU_SEPARATOR,
    MV_MENU_ITEM("Open..."),
    MV_MENU_ITEM("Save"),
    MV_MENU_ITEM("Save As..."),
    MV_MENU_SEPARATOR,
    MV_MENU_ITEM("Exit"),
};

typedef enum {
    EditMenuIndex_Undo = 0,
} EditMenuIndex;

static MIDSCR edit_menu_items[] = {
    MV_MENU_ITEM("Undo"),
};

static MIDSCR help_menu_items[] = {
    MV_MENU_ITEM("About..."),
};

static MNDSCR menus[] = {
    MV_MENU("File", MN_FILE, file_menu_items),
    MV_MENU("Edit", MN_EDIT, edit_menu_items),
    MV_MENU("Help", MN_HELP, help_menu_items),
};

mv_set_menus(mywindow, "xmastree", menus);


static void exit_action(MSRET *msinfo, int menuid, int itemno) {
    if (mv_document_is_dirty(&xmastree_doc)) {
        if (mv_document_save(&xmastree_doc) == 0) {
            exit(0);
        }
    } else {
        exit(0);
    }
}


static TreeView tree_view;


static void new_action(MSRET *msinfo, int menuid, int itemno) {
    if (mv_document_new(&xmastree_doc)) {
        tree_view_refresh(&tree_view);
    }
}


static void open_action(MSRET *msinfo, int menuid, int itemno) {
    if (mv_document_open(&xmastree_doc)) {
        tree_view_refresh(&tree_view);
    }
}


static void save_action(MSRET *msinfo, int menuid, int itemno) {
    mv_document_save(&xmastree_doc);
}


static void save_as_action(MSRET *msinfo, int menuid, int itemno) {
    mv_document_save_as(&xmastree_doc);
}


static void unknown_action(MSRET *msinfo, int menuid, int itemno) {
}


static void about_action(MSRET *msinfo, int menuid, int itemno) {
    mv_app_show_message_box("xmastree v" APP_VERSION "\r\nBuild xmas trees!", MVMessageBoxType_Info);
}


static void undo_action(MSRET *msinfo, int menuid, int itemno) {
    if (mv_document_undo(&xmastree_doc)) {
        tree_view_refresh(&tree_view);
    }
}


static MVMenuItemAction menu_actions[] = {
    {MN_CLOS, 1, exit_action},
    {MN_FILE, 1, new_action},
    {MN_FILE, 3, open_action},
    {MN_FILE, 4, save_action},
    {MN_FILE, 5, save_as_action},
    {MN_FILE, 7, exit_action},
    {MN_HELP, 1, about_action},
    {MN_EDIT, 1, undo_action},
    {-1, -1, unknown_action}
};


static MVImageGrid toolbox;


static int xmastree_handle_key_event(MVUiEvent *event) {
    int item = event->info.key.character - '1';
    if ((item >= -1) && (item <= 8)) {
        if (item == -1) {
            item = 9;
        }
        mv_image_grid_select(&toolbox, item);
    } else if (event->info.key.character == '\x1A') {
        undo_action((MSRET *)NULL, -1, -1);
    } else {
    }

    return true;
}


static int image_ids[XMAS_NUM_ORNAMENTS] = {
    3, 5, 7, 9, 11, 13, 15, 17, 19, 21
};


static int xmastree_handle_click_event(MVUiEvent *event) {
    if (mv_view_dispatch_click(&toolbox.view, event)) {
        return true;
    }
    if (mv_view_dispatch_click(&tree_view.view, event)) {
        MVUndoItem undo_item = { (void (*)(void *))tree_remove_last_item, &tree };
        mv_document_make_change(&xmastree_doc, &undo_item);
    }

    return true;
}


static void xmastree_action(MVUiEvent *event) {
    switch(event->event_type) {
        case MVUiEventType_KeyPress:
            xmastree_handle_key_event(event);
            break;

        case MVUiEventType_MouseClick:
            xmastree_handle_click_event(event);
            break;
    }
}


static void xmastree_pre_init(int argc, char **argv) {
    if (argc > 2) {
        exit(1);
    }

    _cgfx_setgc(MV_OUTPATH, GRP_PTR, PTR_SLP);
    mv_app_set_theme(&theme);
    mv_image_init("xmastree");

    mv_image_load_resource("1m.i09", 2);
    mv_image_load_resource("1.i09", 3);
    mv_image_load_resource("2m.i09", 4);
    mv_image_load_resource("2.i09", 5);
    mv_image_load_resource("3m.i09", 6);
    mv_image_load_resource("3.i09", 7);
    mv_image_load_resource("4m.i09", 8);
    mv_image_load_resource("4.i09", 9);
    mv_image_load_resource("5m.i09", 10);
    mv_image_load_resource("5.i09", 11);
    mv_image_load_resource("6m.i09", 12);
    mv_image_load_resource("6.i09", 13);
    mv_image_load_resource("7m.i09", 14);
    mv_image_load_resource("7.i09", 15);
    mv_image_load_resource("8m.i09", 16);
    mv_image_load_resource("8.i09", 17);
    mv_image_load_resource("9m.i09", 18);
    mv_image_load_resource("9.i09", 19);
    mv_image_load_resource("10m.i09", 20);
    mv_image_load_resource("10.i09", 21);
    Flush();

    tree_init(&tree);
    mv_document_init(
        &xmastree_doc,
        NULL,
        "tree",
        ".xmt",
        &tree,
        (int (*)(void *, const char *))tree_init,
        (int (*)(void *, const char *))tree_open,
        (int (*)(void *, const char *))tree_save
    );

    if (argc == 2) {
        tree_open(&tree, argv[1]);
        mv_document_opened(&xmastree_doc);
    }
}


static void xmastree_toolbox_item_selected(MVImageGrid *toolbox) {
    tree_view_set_item_id(&tree_view, mv_image_grid_selected(toolbox));
}


static void xmastree_init(void) {
    /* The *MenuIndex_ constants are hand-counted, so they drift if menu items
       or separators are reordered. These checks (compiled out under NDEBUG)
       catch that early. */
    assert(strncmp(file_menu_items[FileMenuIndex_Save]._mittl, "Save", 5) == 0);
    assert(strncmp(edit_menu_items[EditMenuIndex_Undo]._mittl, "Undo", 5) == 0);

    _cgfx_bcolor(MV_OUTPATH, XMAS_BACKGROUND);
    _cgfx_clear(MV_OUTPATH);

    mv_image_grid_init(&toolbox, TOOLBOX_X, TOOLBOX_Y, XMAS_NUM_ORNAMENTS,
                       TOOLBOX_COLUMNS, image_ids, xmastree_toolbox_item_selected);
    tree_view_init(&tree_view, TREE_VIEW_X, TREE_VIEW_Y, TREE_VIEW_WIDTH, TREE_VIEW_HEIGHT,
                   &tree, mv_image_grid_selected(&toolbox), image_ids);
    tree_view_refresh(&tree_view);

    Flush();
}


void xmastree_refresh_menus_action() {
    mv_menu_item_set_enabled(file_menu_items, FileMenuIndex_Save, mv_document_is_dirty(&xmastree_doc));
    mv_menu_item_set_enabled(edit_menu_items, EditMenuIndex_Undo, mv_document_can_undo(&xmastree_doc));
}


int main(int argc, char **argv) {
    return mv_app_run(argc, argv, &mywindow, xmastree_pre_init, xmastree_init,
                      menu_actions, xmastree_refresh_menus_action, xmastree_action);
}
