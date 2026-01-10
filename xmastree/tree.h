#ifndef _TREE_H_
#define _TREE_H_

#include <os.h>

#define TREE_MAX_ITEMS 100
#define TREE_MAX_ITEM_ID 9


typedef struct {
    int x;
    int y;
    int item_id;
} TreeItem;


typedef struct {
    int num_items;
    TreeItem items[TREE_MAX_ITEMS];
} Tree;


extern void tree_init(Tree *tree);
extern error_code tree_open(Tree *tree, const char *filename);
extern error_code tree_save(const Tree *tree, const char *filename);
extern error_code tree_add_item(Tree *tree, const TreeItem *item);
extern void tree_remove_last_item(Tree *tree);
extern int tree_get_item_count(const Tree *tree);

#endif /* _TREE_H_ */