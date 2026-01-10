#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "tree.h"


#define MIN_COORDINATE (-12)


void tree_init(Tree *tree) {
    tree->num_items = 0;
}


static int tree_validate(Tree *tree) {
    if ((tree->num_items < 0) || (tree->num_items > TREE_MAX_ITEMS)) {
        return E$BMHP;
    }
    for (int ii=0; ii<tree->num_items; ii++) {
        TreeItem *item = tree->items + ii;
        if ((item->x < MIN_COORDINATE) || (item->x > 319) || (item->y < MIN_COORDINATE) || (item->y > 199) ||
            (item->item_id < 0) || (item->item_id > TREE_MAX_ITEM_ID)) {
            return E$BMHP;
        }
    }
    return 0;
}


error_code tree_open(Tree *tree, const char *filename) {
    int fd = open(filename, FAP_READ);
    if (fd < 0) {
        return errno;
    }
    int bytes_read = read(fd, (char *)tree, sizeof(Tree));
    int retval;
    if (bytes_read >= 0) {
        retval = tree_validate(tree);
    } else {
        retval = errno;
    }
    close(fd);

    if (retval) {
        tree_init(tree);
    }

    return retval;
}


error_code tree_save(const Tree *tree, const char *filename) {
    int fd = creat(filename, FAP_WRITE);
    if (fd < 0) {
        return errno;
    }
    int bytes_to_write = (char *)(&(tree->items[tree->num_items])) - (char *)tree;
    int bytes_written = write(fd, (const char *)tree, bytes_to_write);
    int retval;
    if (bytes_written < 0) {
        retval = errno;
    } else if (bytes_to_write != bytes_written) {
        retval = E$Write;
    } else {
        retval = 0;
    }
    close(fd);

    return retval;
}


error_code tree_add_item(Tree *tree, const TreeItem *item) {
    if (tree->num_items >= TREE_MAX_ITEMS) {
        return E$MemFul;
    }
    tree->items[tree->num_items++] = *item;
    return 0;
}


void tree_remove_last_item(Tree *tree) {
    if (tree->num_items > 0) {
        --tree->num_items;
    }
}


int tree_get_item_count(const Tree *tree) {
    return tree->num_items;
}
