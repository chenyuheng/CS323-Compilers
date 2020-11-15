#include<stdio.h> 
#include<stdlib.h>
#include<string.h> 
#include<stdarg.h>
extern int yylineno;

typedef struct Node {
    int line_num; 
	char type[32];
	char value[32];
	int children_num;
	struct Node** children;
} Node;

Node* get_node(char *type, char *value, int line_num, int children_num, ...);
void print_tree(Node* root, int height);