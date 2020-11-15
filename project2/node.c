#include"node.h"
/* get a new node. */
Node* get_node(char *type_str, char *value, int line_num, int children_num, ...) {
	Node *node = malloc(sizeof(Node));
	node->line_num = line_num;
	node->children_num = children_num;
	strcpy( node->type_str, type_str );
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
	if (get_primitive_type(type_str)) {
		node->type = get_primitive_type(type_str);
	}
	if (strcmp(type_str, "TYPE") == 0) {
		node->type = get_primitive_type(value);
	}
	return node;
}

/* print the whole tree */
void print_tree(Node* parent, int ind) {
    if (parent == NULL) return;
	if (parent->children == NULL) {
		printf("%*s", ind * 2, "");
		if (strcmp(parent->type_str, "TYPE") && strcmp(parent->type_str,"INT") 
		&& strcmp(parent->type_str,"FLOAT") && strcmp(parent->type_str, "ID") 
		&& strcmp(parent->type_str, "CHAR")) {
			printf("%s\n", parent->type_str);
		} else {
			printf("%s: %s\n", parent->type_str, parent->value);
		}
	} else {
		printf("%*s", ind * 2, "");
		printf("%s (%d)\n", parent->type_str, parent->line_num);
		int children_num_remains = parent->children_num;
		//printf("num: %d\n", parent->children_num);
		while (children_num_remains--) {
			print_tree(parent->children[parent->children_num - children_num_remains - 1], ind + 1);
		}
	}
}

Type* get_primitive_type(char* type_str) {
	Type* type = (Type*)malloc(sizeof(Type));
	type->category = PRIMITIVE;
	if (strcmp(type_str, "INT") == 0 || strcmp(type_str, "int") == 0) {
		type->primitive = P_INT;
	} else if (strcmp(type_str, "FLOAT") == 0 || strcmp(type_str, "float") == 0) {
		type->primitive = P_FLOAT;
	} else if (strcmp(type_str, "CHAR") == 0 || strcmp(type_str, "char") == 0) {
		type->primitive = P_CHAR;
	} else {
		return NULL;
	}
	return type;
}

Type* get_structure_type(char* name) {
	Type* type = (Type*)malloc(sizeof(Type));
	type->category = STRUCTURE;
	strcpy(type->name, name);
	return type;
}