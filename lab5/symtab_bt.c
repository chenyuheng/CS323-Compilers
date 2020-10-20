#include "symtab.h"

/*
 * symbol table type, binary tree impl
 */
struct symtab {
    entry entry;
    struct symtab *left, *right;
};

// ************************************************************
//    Your implementation goes here
// ************************************************************

symtab *symtab_init(){
    symtab *self = malloc(sizeof(symtab));
    memset(self, '\0', sizeof(symtab));
    self->left = NULL;
    self->right = NULL;
    return self;
}

symtab* new_node(char* key, VAL_T value) {
    symtab* temp = malloc(sizeof(symtab));
    memset(temp, '\0', sizeof(symtab));
    temp->left = NULL;
    temp->right = NULL;
    entry_init(&temp->entry, key, value);
    return temp;
}

symtab* insert(symtab* node, char* key, VAL_T value) {
    if (node == NULL) return new_node(key, value);
    int cmp = strcmp(node->entry.key, key);
    if (cmp < 0) {
        symtab* temp = insert(node->left, key, value);
        if (temp == NULL) {
            return NULL;
        }
        node->left = temp;
    } else if (cmp > 0) {
        symtab* temp = insert(node->right, key, value);
        if (temp == NULL) {
            return NULL;
        }
        node->right = temp;
    } else {
        return NULL;
    }
    return node;
}

int symtab_insert(symtab *self, char *key, VAL_T value){
    symtab* temp = insert(self, key, value);
    if (temp == NULL) {
        return 0;
    }
    return 1;
}

symtab* lookup(symtab* root, char* key) {
    if (root == NULL) return NULL;
    int cmp = strcmp(root->entry.key, key);
    if (cmp == 0) 
        return root;
    if (cmp > 0) 
        return lookup(root->right, key);
    return lookup(root->left, key);
}

VAL_T symtab_lookup(symtab *self, char *key){
    symtab* temp = lookup(self, key);
    if (temp == NULL) {
        return -1;
    }
    return temp->entry.value;
}

symtab* min_value_node(symtab* node) {
    symtab* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

symtab* delete(symtab* node, char* key) {
    if (node == NULL) return node;
    int cmp = strcmp(node->entry.key, key);
    if (cmp < 0) node->left = delete(node->left, key);
    if (cmp > 0) node->right = delete(node->right, key);
    if (cmp == 0) {
        if (node->left == NULL) {
            symtab* temp = node->right;
            free(node);
            return temp;
        } else if (node->right == NULL) {
            symtab* temp = node->left;
            free(node);
            return temp;
        }
        symtab* temp = min_value_node(node->right);
        node->entry = temp->entry;
        node->right = delete(node->right, temp->entry.key);
    }
    return node;
}

int symtab_remove(symtab *self, char *key){
    symtab* temp = lookup(self, key);
    if (temp == NULL) {
        return 0;
    }
    self = delete(self, key);
    return 1;
}
