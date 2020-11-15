#include"node.h"
/* get a new node. */
Node* get_node(char *type, char *value, int line_num, int children_num, ...) {
	Node *node = malloc(sizeof(Node));
	node->line_num = line_num;
	node->children_num = children_num;
	strcpy( node->type, type );
	strcpy( node->value, value );
	if (children_num) {
		node->children = malloc(sizeof(Node*) * children_num);
		va_list args;
		va_start(args, children_num);
		//printf("%s: line_num: %d, children num: %d\n", type, line_num, children_num);
		for (Node **p = node->children; children_num--; p++) {
			Node* temp = va_arg(args, Node*);
			*p = temp;
		}
	} else {
		node->children = NULL;
	}
	return node;
}

/* print the whole tree */
void print_tree(Node* parent, int ind) {
    if (parent == NULL) return;
	if (parent->children == NULL) {
		printf("%*s", ind * 2, "");
		if (strcmp(parent->type, "TYPE") && strcmp(parent->type,"INT") && strcmp(parent->type,"FLOAT") && strcmp(parent->type, "ID") && strcmp(parent->type, "CHAR")) {
			printf("%s\n", parent->type);
		} else {
			printf("%s: %s\n", parent->type, parent->value);
		}
	} else {
		printf("%*s", ind * 2, "");
		printf("%s (%d)\n", parent->type, parent->line_num);
		int children_num_remains = parent->children_num;
		//printf("num: %d\n", parent->children_num);
		while (children_num_remains--) {
			print_tree(parent->children[parent->children_num - children_num_remains - 1], ind + 1);
		}
	}
	
	
}