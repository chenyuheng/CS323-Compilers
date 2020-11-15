#ifndef NODE_H
#define NODE_H
#include<stdio.h> 
#include<stdlib.h>
#include<string.h> 
#include<stdarg.h>
extern int yylineno;

typedef struct Node {
    int line_num; 
	char type_str[32];
	char value[32];
	int children_num;
	struct Node** children;
	struct Type* type;
} Node;

typedef struct Type {
	char name[32];
	enum {PRIMITIVE, ARRAY, STRUCTURE} category;
	union {
		enum {P_INT, P_FLOAT, P_CHAR } primitive;
		struct Array *array;
		struct FieldList *structure;
	};
} Type;

typedef struct Array {
	struct Type *base;
	int size;
} Array;

typedef struct FieldList {
	char name[32];
	struct Type *type;
	struct FieldList *next;
} FieldList;


Node* get_node(char *type_str, char *value, int line_num, int children_num, ...);
void print_tree(Node* root, int height);
Type* get_primitive_type(char* primitive_type);
Type* get_structure_type(char* name);
void processExtDef(Node* root);
char* getVarDecId(Node* VarDec);
#endif