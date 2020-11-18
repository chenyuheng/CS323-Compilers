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
	enum {PRIMITIVE, ARRAY, STRUCTURE, FUNCTION} category;
	union {
		enum {P_INT, P_FLOAT, P_CHAR } primitive;
		struct Array *array;
		struct FieldList *structure;
		struct Function *function;
	};
} Type;

typedef struct Array {
	struct Type *base;
	int size;
} Array;

typedef struct Function {
	char name[32];
	struct Type* returnType;
	struct FieldList* args;
} Function;

typedef struct FieldList {
	char name[32];
	struct Type *type;
	struct FieldList *next;
} FieldList;


Node* get_node(char *type_str, char *value, int line_num, int children_num, ...);
void print_tree(Node* root, int height);
Type* get_primitive_type(char* primitive_type);
Type* get_structure_type(Node* root);
void processExtDefVar(Node* root);
void processExtDefFun(Node* root);
Node* getVarDecId(Node* VarDec);
void processDef(Node* root);
void processVarList(Node* VarList);
void processBinaryArithmeticOperation(Node* Exp);
void print_type(Type* type, int is_end);
int typecmp(Type* t1, Type* t2);
#endif