#ifndef SYMTAB_H
#define SYMTAB_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"node.h"

#define KEY_LEN 32
#define VAL_T Type*

typedef struct entry {
    char key[KEY_LEN+1];
    VAL_T value;
} entry;

typedef struct symtab {
    entry entry;
    struct symtab *next;
} symtab;
/* symbol table entry, only used internally */



// init a single symbol table
symtab *symtab_init();

// insert a key-value pair to the table
// if insert success, return 1, otherwise 0
int symtab_insert(symtab*, char*, VAL_T);

// lookup the value of a specific key
// return -1 if not found
VAL_T symtab_lookup(symtab*, char*);

// remove a key-value pair from the table
// if remove success, return 1, otherwise 0
int symtab_remove(symtab*, char*);

VAL_T symtab_stack_look(symtab**, int, char*);

#endif  /* SYMTAB_H */
