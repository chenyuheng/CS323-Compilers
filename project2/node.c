#include"node.h"
#include"symtab.h"
extern symtab* global_symtab;
extern symtab* function_symtab;
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

Type* getVarDecType(Node* VarDec, Type* base_type) {
	if (VarDec->children_num == 1) {
		return base_type;
	}
	Type* type = (Type*)malloc(sizeof(Type));
	type->category = ARRAY;
	Array* array = (Array*)malloc(sizeof(Array));
	array->base = getVarDecType(VarDec->children[0], base_type);
	array->size = atoi(VarDec->children[2]->value);
	type->array = array;
	return type;
}

Node* getVarDecId(Node* VarDec) {
	if (VarDec->children_num == 1) {
		return VarDec->children[0];
	}
	return getVarDecId(VarDec->children[0]);
}

void processExtDefVar(Node* root) {
	Type* base_type = root->children[0]->type;
	Node* ExtDecList = root->children[1];
	while (1) {
		Node* VarDec = ExtDecList->children[0];
		VarDec->type = getVarDecType(VarDec, base_type);
		Node* varId = getVarDecId(VarDec);
		int insertion = symtab_insert(global_symtab, varId->value, VarDec->type);
        if (insertion == -1) {
            printf("Error type 3 at Line %d: variable %s is redefined in the same scope.\n",
            varId->line_num, VarDec->children[0]->value);
        }
		if (ExtDecList->children_num == 1) {
            break;
        }
        ExtDecList = ExtDecList->children[2];
	}
}

void processExtDefFun(Node* root) {
	Type* return_type = root->children[0]->type;
	Type* type = (Type*)malloc(sizeof(Type));
	type->category = FUNCTION;
	Node* FunDec = root->children[1];
	Node* ID = FunDec->children[0];
	Function* function = (Function*)malloc(sizeof(Function));
	type->function = function;
	function->returnType = return_type;
	strcpy(function->name, ID->value);
	strcpy(type->name, ID->value);
	if (symtab_lookup(function_symtab, ID->value) != NULL) {
		printf("Error type 4 at Line %d: function %s is redefined.\n", ID->line_num, ID->value);
		return;
	}
	if (FunDec->children_num == 3) {
		symtab_insert(function_symtab, ID->value, type);
		return;
	}
	Node* VarList = FunDec->children[2];
	FieldList* currentField = (FieldList*)malloc(sizeof(FieldList));
	function->args = currentField;
	while (1) {
		Node* ParamDec = VarList->children[0];
		strcpy(currentField->name, getVarDecId(ParamDec->children[1])->value);
		currentField->type = ParamDec->children[0]->type;
		if (VarList->children_num == 1) {
			break;
		}
		VarList = VarList->children[2];
		FieldList* nextField = (FieldList*)malloc(sizeof(FieldList));
		currentField->next = nextField;
		currentField = nextField;
	}
	symtab_insert(function_symtab, ID->value, type);
}

void processDef(Node* root) {
	Type* base_type = root->children[0]->type;
	Node* DecList = root->children[1];
	while (1) {
		Node* Dec = DecList->children[0];
		Node* VarDec = Dec->children[0];
		Dec->type = getVarDecType(VarDec, base_type);
		Node* varId = getVarDecId(VarDec);
		int insertion = symtab_insert(global_symtab, varId->value, Dec->type);
        if (insertion == -1) {
            printf("Error type 3 at Line %d: variable %s is redefined in the same scope.\n",
            varId->line_num, VarDec->children[0]->value);
        }
		if (Dec->children_num == 3) {
			// TODO: add assignment support
		}
		if (DecList->children_num == 1) {
            break;
        }
        DecList = DecList->children[2];
	}
}

void processVarList(Node* VarList) {
	while (1) {
		Node* ParamDec = VarList->children[0];
		int insertion = symtab_insert(global_symtab, 
			getVarDecId(ParamDec->children[1])->value, getVarDecType(ParamDec->children[1], ParamDec->children[0]->type));
		if (insertion == -1) {
			printf("Error type 3 at Line %d: variable %s is redefined in the same scope.\n",
					getVarDecId(ParamDec->children[1])->line_num, getVarDecId(ParamDec->children[1])->value);
		}
		if (VarList->children_num != 3) {
			break;
		}
		VarList = VarList->children[2];
	}
}

void processBinaryArithmeticOperation(Node* Exp) {
	Node* left = Exp->children[0];
	Node* op = Exp->children[1];
	Node* right = Exp->children[2];
	if (left->type == NULL || right->type == NULL) {
		return;
	}
	if (left->type->category != PRIMITIVE) {
		printf("Error type 7 at Line %d: binary operation on non-number variables\n", left->line_num);
	}
	if (right->type->category != PRIMITIVE) {
		printf("Error type 7 at Line %d: binary operation on non-number variables\n", right->line_num);
	}
	if (left->type != PRIMITIVE || right->type != PRIMITIVE) {
		return;
	}
	if (left->type->primitive == P_CHAR) {
		printf("Error type 7 at Line %d: binary operation on non-number variables\n", left->line_num);
	}
	if (right->type->primitive == P_CHAR) {
		printf("Error type 7 at Line %d: binary operation on non-number variables\n", right->line_num);
	}
	if (left->type->primitive == P_CHAR || right->type->primitive == P_CHAR) {
		return;
	}
	Exp->type = (Type*)malloc(sizeof(Type));
	Exp->type->category = PRIMITIVE;
	if (!(strcmp(op->type_str, "LT") && strcmp(op->type_str, "LE") && strcmp(op->type_str, "GT") && 
		strcmp(op->type_str, "GE") && strcmp(op->type_str, "NE") && strcmp(op->type_str, "EQ"))) {
		Exp->type->primitive = P_INT;
	} else if (left->type->primitive == P_INT && right->type->primitive == P_INT) {
		Exp->type->primitive = P_INT;
	} else {
		Exp->type->primitive = P_FLOAT;
	}
}

int typecmp(Type* t1, Type* t2) {
	if (t1 == NULL || t2 == NULL) {
		return 0;
	}
	if (t1->category != t2->category) {
		return 0;
	}
	if (t1->category == PRIMITIVE) {
		return t1->primitive == t2->primitive;
	}
	if (t1->category == ARRAY) {
		if (t1->array->size != t2->array->size) {
			return 0;
		}
		return typecmp(t1->array->base, t2->array->base) == 0;
	}
	return strcmp(t1->name, t2->name); // TODO: structual equivelent

}

void print_type(Type* type, int is_end) {
	if (type->category == PRIMITIVE) {
		if (type->primitive == P_INT) printf("int");
		if (type->primitive == P_FLOAT) printf("float");
		if (type->primitive == P_CHAR) printf("char");
	} else if (type->category == STRUCTURE) {
		printf("struct %s", type->name);
	} else if (type->category == ARRAY) {
		print_type(type->array->base, 0);
		printf("[%d]", type->array->size);
	} else if (type->category == FUNCTION) {
		print_type(type->function->returnType, 0);
		printf(" %s(", type->name);
		FieldList* currentField = type->function->args;
		while (currentField) {
			print_type(currentField->type, 0);
			printf(" %s, ", currentField->name);
			currentField = currentField->next;
		}
		printf(")");
	}
	if (is_end) {
		printf("\n");
	}
}