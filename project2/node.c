#include"node.h"
#include"symtab.h"
extern symtab* global_symtab;
extern symtab* function_symtab;
extern symtab* structure_symtab;
extern symtab** symtab_stack;
extern int sp;
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

Type* get_structure_type(Node* root) {
	Type* type = (Type*)malloc(sizeof(Type));
	type->category = STRUCTURE;
	strcpy(type->name, root->children[1]->value);
	FieldList* currentField = (FieldList*)malloc(sizeof(FieldList));
	type->structure = currentField;
	Node* DefList = root->children[3];
	while (1) {
		if (DefList == NULL) {
			break;
		}
		Node* Def = DefList->children[0];
		Node* DecList = Def->children[1];
		while (1) {
			Node* Dec = DecList->children[0];
			strcpy(currentField->name, getVarDecId(Dec->children[0])->value);
			currentField->type = getVarDecType(Dec->children[0], Def->children[0]->type);
			currentField->next = (FieldList*)malloc(sizeof(FieldList));
			currentField = currentField->next;
			if (DecList->children_num == 1) {
				break;
			}
			DecList = DecList->children[2];
		}
		DefList = DefList->children[1];
	}
	currentField = type->structure;
	while (1) {
		if (currentField->next->type == NULL) {
			currentField->next = NULL;
			break;
		}
		currentField = currentField->next;
	}
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
		currentField->type = getVarDecType(ParamDec->children[1], ParamDec->children[0]->type);
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

void processStructSpecifier(Node* StructSpecifier) {
	Type* type = get_structure_type(StructSpecifier);
	int insertion = symtab_insert(structure_symtab, type->name, type);
	if (insertion == -1) {
		printf("Error type 15 at Line %d: redefine the same structure type\n", StructSpecifier->children[1]->line_num);
	}
	StructSpecifier->type = type;
}

void processDef(Node* root) {
	Type* base_type = root->children[0]->type;
	Node* DecList = root->children[1];
	while (1) {
		Node* Dec = DecList->children[0];
		Node* VarDec = Dec->children[0];
		Dec->type = getVarDecType(VarDec, base_type);
		Node* varId = getVarDecId(VarDec);
		strcpy(Dec->value, varId->value);
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
	if (type == NULL) {
		return;
	}
	if (type->category == PRIMITIVE) {
		if (type->primitive == P_INT) printf("int");
		if (type->primitive == P_FLOAT) printf("float");
		if (type->primitive == P_CHAR) printf("char");
	} else if (type->category == STRUCTURE) {
		printf("struct %s: {", type->name);
		if (type->structure != NULL) {
			FieldList* currentField = type->structure;
			while (currentField) {
				printf("%s: ", currentField->name);
				print_type(currentField->type, 0);
				printf(", ");
				currentField = currentField->next;
			}
		}
		printf("}");
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

void checkAndSetArrayType(Type* arr) {
	Type* current_base = arr;
	while(current_base->array->base->category == ARRAY) {
		current_base = current_base->array->base;
	}
	if (current_base->array->base->category == STRUCTURE) {
		Type* current_array_type = symtab_lookup(structure_symtab, current_base->array->base->name);
		if (current_array_type != NULL) {
			current_base->array->base = current_array_type;
		} else {
		printf("not defined structure type\n");
		}
	}
}

void completeSymbolTableVariableNode(symtab* temp_symtab) {
	Type* current_type = temp_symtab->entry.value;
	if (current_type->category == STRUCTURE) {
		Type* struct_type = symtab_lookup(structure_symtab, current_type->name);
		if (struct_type != NULL) {
			temp_symtab->entry.value = struct_type;
		} else {
			printf("not defined structure type\n");
		}
	} else if (current_type->category == ARRAY) {
		checkAndSetArrayType(current_type);
	}
}

void checkFieldlist(FieldList* start_point) {
	FieldList* current_fieldlist = start_point;
	while (current_fieldlist != NULL) {
		if (current_fieldlist->type->category == STRUCTURE) {
			Type* field_type = symtab_lookup(structure_symtab, current_fieldlist->type->name);
			if (field_type != NULL) { 
				current_fieldlist->type = field_type;
			} else {
				printf("not defined structure type of return\n");
			}
		} else if (current_fieldlist->type->category == ARRAY) {
			checkAndSetArrayType(current_fieldlist->type);
		}
		current_fieldlist = current_fieldlist->next;
	}
}

void completeSymbolTableFunctionNode(Function* function) {
	if (function->returnType->category == STRUCTURE) {
		Type* return_type = symtab_lookup(structure_symtab, function->returnType->name);
		if (return_type != NULL) {
			function->returnType = return_type;
		} else {
			printf("not defined structure type of return\n");
		}
	}
	checkFieldlist(function->args);
}


void completeSymbolTable() {
	    symtab* temp_symtab = global_symtab->next;
        while (temp_symtab != NULL) {
			completeSymbolTableVariableNode(temp_symtab);
            temp_symtab = temp_symtab->next;
        }        
        temp_symtab = function_symtab->next;
		while (temp_symtab != NULL) {
			completeSymbolTableFunctionNode(temp_symtab->entry.value->function);
			temp_symtab = temp_symtab->next;
		}
		temp_symtab = structure_symtab->next;
		while (temp_symtab != NULL) {
			checkFieldlist(temp_symtab->entry.value->structure);
			temp_symtab = temp_symtab->next;
		}
}

void pushSymtab() {
	sp++;
	symtab_stack[sp] = symtab_init();
}

int is_int(Type* type) {
	if (type->category != PRIMITIVE) return 0;
	return type->primitive == P_INT;
}

int is_float(Type* type) {
	if (type->category != PRIMITIVE) return 0;
	return type->primitive == P_FLOAT;
}

int is_char(Type* type) {
	if (type->category != PRIMITIVE) return 0;
	return type->primitive == P_CHAR;
}

Type* get_int() {
	Type* type = (Type*)malloc(sizeof(Type));
	type->category = PRIMITIVE;
	type->primitive = P_INT;
	return type;
}

Type* get_float() {
	Type* type = (Type*)malloc(sizeof(Type));
	type->category = PRIMITIVE;
	type->primitive = P_FLOAT;
	return type;
}

Type* get_char() {
	Type* type = (Type*)malloc(sizeof(Type));
	type->category = PRIMITIVE;
	type->primitive = P_CHAR;
	return type;
}

int equal_type(Type* t1, Type* t2) {
	if (t1->category != t2->category) return 0;
	if (t1->category == PRIMITIVE) {
		return t1->primitive == t2->primitive;
	}
	if (t1->category == ARRAY) {
		if (t1->array->size != t2->array->size) return 0;
		return equal_type(t1->array->base, t2->array->base);
	}
	if (t1->category == STRUCTURE) {
		FieldList* f1 = t1->structure;
		FieldList* f2 = t2->structure;
		while (1) {
			if (!equal_type(f1->type, f2->type)) return 0;
			f1 = f1->next;
			f2 = f2->next;
			if (f1 == NULL && f2 == NULL) return 1;
			if (f1 == NULL || f2 == NULL) return 0;
		}
	}
}

void print_error(int type_num, int line_num, char* msg) {
	printf("Error type %d at Line %d: %s\n", type_num, line_num, msg);
}

Type* search_fieldlist(FieldList* f, char* key) {
	while (f) {
		if (!strcmp(f->name, key)) return f->type;
		f = f->next;
	}
	return NULL;
}

void traverse(Node* root) {
	for (int i = 0; i < root->children_num; i++) {
		traverse(root->children[i]);
	}
	if (!strcmp(root->type_str, "Exp")) {
		if (root->children_num == 3) {
			// 分为两类：Exp op Exp 与其他
			if (!strcmp(root->children[0]->type_str,'Exp')) {
				Node* e1 = root->children[0];
				Node* op = root->children[1];
				Node* e2 = root->children[2];
				if(e1->type == NULL || e2->type == NULL) return;
				if (!strcmp(op->type_str, "ASSIGN")) {
					if (!equal_type(e1->type, e2->type)) {
						print_error(5, e1->line_num, " unmatching type on both sides of assignment");
						return;
					}
					root->type = e1->type;
				} else if (!strcmp(op->type_str, "DOT")) {
					if (e1->type->category != STRUCTURE) {
						print_error(13, e1->line_num, "accessing with non-struct variable");
						return;
					}
					Type* new_type = search_fieldlist(e1->type->structure, e2->value);
					if (new_type == NULL) {
						print_error(14, e1->line_num, "no such member");
						return;
					}
					root->type = new_type;
				} else if (!strcmp(op->type_str, "AND") || !strcmp(op->type_str, "OR")) {
					if (!is_int(e1->type) || !is_int(e2->type)) {
						printf("break assumption 2\n");
					}
					root->type = get_int();
				} else if (!strcmp(op->type_str, "")) {
					
				}
			} else {

			}
		}
	}
}