#include"node.h"
#include"symtab.h"
extern symtab* global_symtab;
extern symtab* function_symtab;
extern symtab* structure_symtab;
extern symtab** symtab_stack;
extern int sp;
extern int unique_counter;
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
	if (!strcmp(type_str, "TYPE")) {
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
		print_error(4, ID->line_num, ID->value);
		return;
	}
	function->args_num = 0;
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
		function->args_num++;
		currentField->type = getVarDecType(ParamDec->children[1], ParamDec->children[0]->type);
		// printf("%s:\n", FunDec->children[0]->value);
		// print_type(ParamDec->children[0]->type, 1);
		// print_type(currentField->type, 1);
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

// void processDef(Node* root) {
// 	Type* base_type = root->children[0]->type;
// 	Node* DecList = root->children[1];
// 	while (1) {
// 		Node* Dec = DecList->children[0];
// 		Node* VarDec = Dec->children[0];
// 		Dec->type = getVarDecType(VarDec, base_type);
// 		Node* varId = getVarDecId(VarDec);
// 		strcpy(Dec->value, varId->value);
// 		int insertion = symtab_insert(global_symtab, varId->value, Dec->type);
//         if (insertion == -1) {
//             printf("Error type 3 at Line %d: variable %s is redefined in the same scope.\n",
//             varId->line_num, VarDec->children[0]->value);
//         }
// 		if (Dec->children_num == 3) {
// 			// TODO: add assignment support
// 		}
// 		if (DecList->children_num == 1) {
//             break;
//         }
//         DecList = DecList->children[2];
// 	}
// }

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

// 调试用
void print_type(Type* type, int is_end) {
	if (type == NULL) {
		printf("NULL type\n");
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
				// if (!equal_type(currentField->type, type)) {
				// 	print_type(currentField->type, 0);
				// }
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
		printf(")\t%d", type->function->args_num);
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

int is_number(Type* type) {
	return is_int(type) || is_float(type);
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

Type* get_result_type(Type* t1, Type* t2) {
	if (!is_number(t1) || !is_number(t2)) return NULL;
	if (is_int(t1) && is_int(t2)) return get_int();
	return get_float();
}

int equal_type(Type* t1, Type* t2) {
	if (t1 == NULL || t2 == NULL) return 0;
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
			if (f1->type->category != STRUCTURE) {
				if (!equal_type(f1->type, f2->type)) return 0;
			} else if (strcmp(f1->type->name, f2->type->name)) {
				return 0;
			}
			f1 = f1->next;
			f2 = f2->next;
			if (f1 == NULL && f2 == NULL) return 1;
			if (f1 == NULL || f2 == NULL) return 0;
		}
	}
}

const char * msg_arr[] = {
	"enpty string",
    "undefined variable: ",
    "undefined function: ",
    "redefine variable: ",
	"redefine function: ",
	"unmatching type on both sides of assignment",
	"left side in assignment is rvalue",
	"binary operation on non-number variables",
	"incompatiable return type",
	"invalid argument for function, ",
	"indexing on non-array variable",
	"invoking non-function variable: ",
	"indexing by non-integer",
	"accessing with non-struct variable",
	"no such member: ",
	"redefine struct: "
};

void print_error(int type_num, int line_num, char* msg) {
	printf("Error type %d at Line %d: %s%s\n", type_num, line_num, msg_arr[type_num], msg);
}

Type* search_fieldlist(FieldList* f, char* key) {
	while (f) {
		if (!strcmp(f->name, key)) return f->type;
		f = f->next;
	}
	return NULL;
}

int get_fieldlist_size(FieldList* f) {
	int n = 0;
	while (f != NULL) {
		n++;
		f = f->next;
	}
	return n;
}

/* 
return actual number if not match in number
return -1 if match in number but not match in type
return 0 if equal
*/
int field_list_cmp(FieldList* expected, FieldList* actual) {
	if (get_fieldlist_size(expected) != get_fieldlist_size(actual)) {
		return get_fieldlist_size(actual);
	}
	while (1) {
		if (expected == NULL && actual == NULL) break;
		if (!equal_type(expected->type, actual->type)) return -1;
		expected = expected->next;
		actual = actual->next;
	}
	return 0;
}

FieldList* get_fieldlist_from_Args(Node* Args) {
	FieldList* start = (FieldList*)malloc(sizeof(FieldList));
	start->type = Args->children[0]->type;
	FieldList* currentField = start;
	while (Args->children_num == 3) {
		Args = Args->children[2];
		FieldList* nextField = (FieldList*)malloc(sizeof(FieldList));
		nextField->type = Args->children[0]->type;
		currentField->next = nextField;
		currentField = nextField;
	}
	return start;
}

int is_left_value(Node* Exp) {
	if (Exp->children_num == 1) {
		if (strcmp(Exp->children[0]->type_str, "ID")) return 0;
	} else if (Exp->children_num == 3) {
		if (strcmp(Exp->children[1]->type_str, "DOT")) return 0;
	} else if (Exp->children_num == 4) {
		if (strcmp(Exp->children[0]->type_str, "Exp")) return 0;
	} else {
		return 0;
	}
	return 1;
}

void traverse_Exp(Node* root) {
	if (root->children_num == 4) {
		if (!strcmp(root->children[0]->type_str, "ID")) {
			Type* function = symtab_lookup(function_symtab, root->children[0]->value);
			if (function == NULL) {
				Type* variable = symtab_stack_look(symtab_stack, sp, root->children[0]->value);
				Type* structure = symtab_lookup(structure_symtab, root->children[0]->value);
				if (variable != NULL || structure != NULL) {
					print_error(11, root->line_num, root->children[0]->value);
					return;
				} else {
					print_error(2, root->line_num, "");
					return;
				}
			}
			FieldList* args = get_fieldlist_from_Args(root->children[2]);
			int cmp_result = field_list_cmp(function->function->args, args);
			if (cmp_result == -1) {
				print_error(9, root->line_num, "type(s) not compatible");
			} else if (cmp_result == 0) {
				root->type = function->function->returnType;
			} else {
				char msg[50];
				sprintf(msg, "expected number %d, actulal number %d", function->function->args_num, cmp_result);
				print_error(9, root->line_num, msg);
			}
		} else if (!strcmp(root->children[0]->type_str, "Exp")) {
			if (root->children[0]->type == NULL || root->children[2]->type == NULL) return;
			if (root->children[0]->type->category != ARRAY) {
				print_error(10, root->children[0]->line_num, "");
			}
			if (!is_int(root->children[2]->type)) {
				print_error(12, root->children[2]->line_num, "");
			}
			if (root->children[0]->type->category != ARRAY || !is_int(root->children[2]->type)) {
				return;
			}
			root->type = root->children[0]->type->array->base;
		}
	} else if (root->children_num == 3) {
		// 分为两类：Exp op Exp 与其他
		if (!strcmp(root->children[0]->type_str,"Exp")) {
			Node* e1 = root->children[0];
			Node* op = root->children[1];
			Node* e2 = root->children[2];
			if (!strcmp(op->type_str, "DOT")) {
				if (e1->type->category != STRUCTURE) {
					print_error(13, e1->line_num, "");
					return;
				}
				Type* new_type = search_fieldlist(e1->type->structure, e2->value);
				if (new_type == NULL) {
					print_error(14, e1->line_num, e2->value);
					return;
				}
				root->type = new_type;
				return;
			} else if (!strcmp(op->type_str, "ASSIGN")) {
				if(e1->type == NULL || e2->type == NULL) {
					print_error(5, root->line_num, "");
					return;
				}
				if (!is_left_value(root->children[0])) {
					print_error(6, root->line_num, "");
				}
				if (!equal_type(e1->type, e2->type)) {
					print_error(5, e1->line_num, "");
				}
				if (!is_left_value(root) || !equal_type(e1->type, e2->type)) {
					return;
				}
				root->type = e1->type;
			}
			// printf(">>>>%s<<<\n", op->type_str);
			// print_type(e1->type, 1);
			// print_type(e2->type, 1);
			if(e1->type == NULL || e2->type == NULL) return;
			//printf("<><>\n");
			if (!strcmp(op->type_str, "AND") || !strcmp(op->type_str, "OR")) {
				if (!is_int(e1->type) || !is_int(e2->type)) {
					printf("break assumption 2\n");
					return;
				}
				root->type = get_int();
			} else if (!strcmp(op->type_str, "LT") || !strcmp(op->type_str, "LE")
					|| !strcmp(op->type_str, "GT") || !strcmp(op->type_str, "GE")
					|| !strcmp(op->type_str, "NE") || !strcmp(op->type_str, "EQ")) {
				if (!is_number(e1->type) || !is_number(e2->type)) {
					print_error(7, e1->line_num, "");
					return;
				}
				root->type = get_int();
			} else if (!strcmp(op->type_str, "PLUS") || !strcmp(op->type_str, "MINUS")
					|| !strcmp(op->type_str, "MUL") || !strcmp(op->type_str, "DIV")) {
				Type* result_type = get_result_type(e1->type, e2->type);
				if (result_type == NULL) {
					print_error(7, e1->line_num, "");
					return;
				}
				root->type = result_type;
			}
		} else if (!strcmp(root->children[0]->type_str,"LP")) {
			root->type = root->children[1]->type;
		} else if (!strcmp(root->children[0]->type_str, "ID")) {
			Type* function = symtab_lookup(function_symtab, root->children[0]->value);
			if (function == NULL) {
				Type* variable = symtab_stack_look(symtab_stack, sp, root->children[0]->value);
				Type* structure = symtab_lookup(structure_symtab, root->children[0]->value);
				if (variable != NULL || structure != NULL) {
					print_error(11, root->line_num, root->children[0]->value);
					return;
				} else {
					print_error(2, root->line_num, "");
					return;
				}
			}
			if (function->function->args != NULL) {
				print_error(9, root->line_num, "");
				return;
			}
			root->type = function->function->returnType;
		}
	} else if (root->children_num == 2) {
		if (!strcmp(root->children[0]->type_str, "MINUS") || !strcmp(root->children[0]->type_str, "NOT")) {
			if (!is_number(root->children[1]->type)) {
				print_error(7, root->line_num, "");
				return;
			}
			root->type = root->children[1]->type;
		}
	} else if (root->children_num == 1) {
		if (!strcmp(root->children[0]->type_str, "ID")) {
			Type* result_type = symtab_stack_look(symtab_stack, sp, root->children[0]->value);
			if (result_type == NULL) {
				print_error(1, root->line_num, root->children[0]->value);
				return;
			}
			if (result_type->category == STRUCTURE) {
				Type* structure_type = symtab_lookup(structure_symtab, result_type->name);
				if (structure_type == NULL) {
					printf("Undefined structure type at line %d\n", root->line_num);
				} else {
					result_type->structure = structure_type->structure;
				}
			}
			root->type = result_type;
		} else {
			root->type = root->children[0]->type;
		}
	}
}


void processDef(Node* Def) {
	Type* base_type = Def->children[0]->type;
	Node* DecList = Def->children[1];
	while (1) {
		Node* VarDec = DecList->children[0]->children[0];
		VarDec->type = getVarDecType(VarDec, base_type);
		Node* varId = getVarDecId(VarDec);
		int insertion = symtab_insert(symtab_stack[sp], varId->value, VarDec->type);
        if (insertion == -1) {
			print_error(3, varId->line_num, VarDec->children[0]->value);
        } else {
			//printf("inserted %s in stack %d\n", varId->value, sp);
		}
		if (DecList->children[0]->children_num == 3) {
			Node* Exp = DecList->children[0]->children[2];
			if (!equal_type(Exp->type, VarDec->type)) {
				print_error(5, Exp->line_num, "");
			}
		}
		if (DecList->children_num == 1) {
            break;
        }
        DecList = DecList->children[2];
	}
}

int func_flag;

void processParamDec(Node* ParamDec) {
	Type* base_type = ParamDec->children[0]->type;
	Type* type = getVarDecType(ParamDec->children[1], base_type);
	Node* ID = getVarDecId(ParamDec->children[1]);
	int insertion = symtab_insert(symtab_stack[sp], ID->value, type);
	if (insertion == 0) {
		print_error(3, ParamDec->line_num, ID->value);
	}
}

Type* current_return_type;

void traverse_Stmt(Node* Stmt) {
	if (Stmt->children_num == 3) {
		if (!strcmp(Stmt->children[0]->type_str, "RETURN")) {
			if (Stmt->children[1]->type == NULL) return;
			if (!equal_type(Stmt->children[1]->type, current_return_type)) {
				print_error(8, Stmt->line_num, "");
				// printf("shold be:");
				// print_type(current_return_type, 1);
			}
		}
	}
}

void processSpecifier(Node* Specifier) {
	if (!strcmp(Specifier->children[0]->type_str, "StructSpecifier")) {
		Type* speciType = symtab_lookup(structure_symtab, Specifier->children[0]->type->name);
		if (speciType == NULL) {
		} else {
			Specifier->type = speciType;
		}
	}
}

void traverse(Node* root, int d) {
	if (root == NULL) return;
	// int dd = d;
	// while (dd--) {
	// 	printf("  ");
	// }
	// printf("%s (%d)\n", root->type_str, root->line_num);
	if (!strcmp(root->type_str, "FunDec")) {
		symtab_stack[++sp] = symtab_init();
		func_flag = 1;
		// print_type(current_return_type, 1);
		Type* function_type = symtab_lookup(function_symtab, root->children[0]->value);
		current_return_type = function_type->function->returnType;
	}

	if (!strcmp(root->type_str, "StructSpecifier") || !strcmp(root->type_str, "CompSt")) {
		if (!func_flag) {
			symtab_stack[++sp] = symtab_init();
		} else {
			func_flag = 0;
		}
		//printf("start %d\n", sp);
	}
	for (int i = 0; i < root->children_num; i++) {
		traverse(root->children[i], d+1);
	}
	if (!strcmp(root->type_str, "StructSpecifier") || !strcmp(root->type_str, "CompSt")) {
		symtab_stack[sp] = NULL;
		sp--;
		//printf("end %d\n", sp--);
	}
	if (!strcmp(root->type_str, "Exp")) {
		traverse_Exp(root);
	} else if (!strcmp(root->type_str, "Def")) {
		processDef(root);
	} else if (!strcmp(root->type_str, "ParamDec")) {
		processParamDec(root);
	} else if (!strcmp(root->type_str, "Stmt")) {
		traverse_Stmt(root);
	} else if (!strcmp(root->type_str, "Specifier")) {
		processSpecifier(root);
	}
}

char* new_place() {
	char* str = (char*)malloc(20*sizeof(char));
	sprintf(str, "temp%d", unique_counter++);
	return str;
}

char* new_label() {
	char* str = (char*)malloc(20*sizeof(char));
	sprintf(str, "lab%d", unique_counter++);
	return str;
}

char* translate_Node(Node* node, int l) {
	char** codes = (char**)malloc((node->children_num) * sizeof(char**) + 8);
	int size = 0;
			//printf("%s: %d(%d)\n", node->type_str, node->children_num, node->line_num);
	for (int i = 0; i < node->children_num; i++) {
		if (!strcmp(node->children[i]->type_str, "Exp")) {
			codes[i] = translate_Exp(node->children[i], new_place());
		} else if (!strcmp(node->children[i]->type_str, "Stmt")) {
			codes[i] = translate_Stmt(node->children[i]);
		} else if (!strcmp(node->children[i]->type_str, "Dec") && node->children[i]->children_num == 3) {
			codes[i] = translate_Dec(node->children[i]);
		} else {
			codes[i] = translate_Node(node->children[i], l + 1);
		}
		size += strlen(codes[i]);
	}
	char* tac = (char*)malloc(size + 100);
	if (!strcmp(node->type_str, "ExtDef")) {
		if (node->children_num == 3) {
			if(!strcmp(node->children[1]->type_str, "FunDec")) {
				sprintf(tac, "FUNCTION %s :\n", node->children[1]->children[0]->value);
			}
		}
	}
	for (int i = 0; i < node->children_num; i++) {
		if (codes[i][0] == '\0') {
			continue;
		}
		strcat(tac, codes[i]);
	}
	return tac;
}


char* translate_Exp(Node* Exp, char* place) {
	if (Exp->children_num == 1) {
		if (!strcmp(Exp->children[0]->type_str, "INT")) {
			char* tac = (char*)malloc(1<<7);
			sprintf(tac, "%s := #%s\n", place, Exp->children[0]->value);
			return tac;
		} else if (!strcmp(Exp->children[0]->type_str, "ID")) {
			char* tac = (char*)malloc(1<<7);
			sprintf(tac, "%s := var%s\n", place, Exp->children[0]->value);
			return tac;
		}
	} else if (Exp->children_num == 2) {
		if (!strcmp(Exp->children[0]->type_str, "MINUS")) {
			char* tp = new_place();
			char* code1 = translate_Exp(Exp->children[1], tp);
			char* code2 = (char*)malloc(strlen(place) + 20);
			sprintf(code2, "%s := #0 - %s\n", place, tp);
			char* tac = (char*)malloc(strlen(code1) + strlen(code2) + 30);
			sprintf(tac, "%s%s", code1, code2);
			return tac;
			}
	} else if (Exp->children_num == 3) {
		if (!strcmp(Exp->children[1]->type_str, "ASSIGN")) {
			char* lv = Exp->children[0]->children[0]->value;
			char* tp = new_place();
			char* code1 = translate_Exp(Exp->children[2], tp);
			char* code2 = (char*)malloc(strlen(lv) + 10);
			sprintf(code2, "var%s := %s\n", lv, tp);
			char* code3 = (char*)malloc(strlen(lv) + strlen(place) + 10);
			sprintf(code3, "%s := var%s\n", place, lv);
			char* tac = (char*)malloc(strlen(code1) + strlen(code2) + strlen(code3) + 10);
			sprintf(tac, "%s%s%s", code1, code2, code3);
			return tac;
		} else if (!strcmp(Exp->children[1]->type_str, "PLUS") || !strcmp(Exp->children[1]->type_str, "MINUS")
			    || !strcmp(Exp->children[1]->type_str, "MUL") || !strcmp(Exp->children[1]->type_str, "DIV")
		) {
			char* t1 = new_place();
			char* t2 = new_place();
			char* code1 = translate_Exp(Exp->children[0], t1);
			char* code2 = translate_Exp(Exp->children[2], t2);
			char* code3 = (char*)malloc(strlen(place) + 30);
			char op = '+';
			if (!strcmp(Exp->children[1]->type_str, "MINUS")) {
				op = '-';
			} else if (!strcmp(Exp->children[1]->type_str, "MUL")) {
				op = '*';
			} else if (!strcmp(Exp->children[1]->type_str, "DIV")) {
				op = '/';
			} 
			sprintf(code3, "%s := %s %c %s\n", place, t1, op, t2);
			char* tac = (char*)malloc(strlen(code1) + strlen(code2) + strlen(code3) + 10);
			sprintf(tac, "%s%s%s", code1, code2, code3);
			return tac;
		} else if (!strcmp(Exp->children[0]->type_str, "ID")) {
			if (!strcmp(Exp->children[0]->value, "read")) {
				char* tac = (char*)malloc(30);
				sprintf(tac, "READ %s\n", place);
				return tac;
			} else {

			}
		} else if (!strcmp(Exp->children[1]->type_str, "Exp")) {
			return translate_Exp(Exp->children[1], place);
		} else if (!strcmp(Exp->children[1]->type_str, "AND") || !strcmp(Exp->children[1]->type_str, "OR")
		 	 	|| !strcmp(Exp->children[1]->type_str, "EQ") || !strcmp(Exp->children[1]->type_str, "NE")
		 		|| !strcmp(Exp->children[1]->type_str, "LT") || !strcmp(Exp->children[1]->type_str, "LE")
		 		|| !strcmp(Exp->children[1]->type_str, "GT") || !strcmp(Exp->children[1]->type_str, "GE")) {
			char* lb1 = new_label();
			char* lb2 = new_label();
			char* code1 = translate_cond_Exp(Exp, lb1, lb2);
			char* tac = (char*)malloc(strlen(code1) + 100);
			sprintf(tac, "%s := #0\n%sLABEL %s\n%s := #1\nLABEL %s\n", place, code1, lb1, place, lb2);
			return tac;
		}
	} else if (Exp->children_num == 4) {
		if (!strcmp(Exp->children[0]->type_str, "ID")) {
			if (!strcmp(Exp->children[0]->value, "write")) {
				char* tp = new_place();
				char* code = translate_Exp(Exp->children[2]->children[0], tp);
				char* tac = (char*)malloc(strlen(code) + 30);
				sprintf(tac, "%sWRITE %s\n", code, tp);
				return tac;
			}
		}
	}
}

char* translate_cond_Exp(Node* Exp, char* lb_t, char* lb_f) {
	if (Exp->children_num == 2) {
		if (!strcmp(Exp->children[0]->type_str, "NOT")) {
			return translate_cond_Exp(Exp, lb_f, lb_t);
		}
	} else if (Exp->children_num == 3) {
		if (!strcmp(Exp->children[1]->type_str, "AND")) {
			char* lb1 = new_label();
			char* code1 = translate_cond_Exp(Exp->children[0], lb1, lb_f);
			char* code2 = (char*)malloc(30);
			sprintf(code2, "LABEL %s\n", lb1);
			char* code3 = translate_cond_Exp(Exp->children[2], lb_t, lb_f);
			char* tac = (char*)malloc(strlen(code1) + strlen(code3) + 50);
			sprintf(tac, "%s%s%s", code1, code2, code3);
			return tac;
		} else if (!strcmp(Exp->children[1]->type_str, "OR")) {
			char* lb1 = new_label();
			char* code1 = translate_cond_Exp(Exp->children[0], lb_t, lb1);
			char* code2 = (char*)malloc(30);
			sprintf(code2, "LABEL %s\n", lb1);
			char* code3 = translate_cond_Exp(Exp->children[2], lb_t, lb_f);
			char* tac = (char*)malloc(strlen(code1) + strlen(code3) + 50);
			sprintf(tac, "%s%s%s", code1, code2, code3);
			return tac;
		} else if (!strcmp(Exp->children[1]->type_str, "EQ") || !strcmp(Exp->children[1]->type_str, "NE") 
			    || !strcmp(Exp->children[1]->type_str, "GT") || !strcmp(Exp->children[1]->type_str, "GE") 
			    || !strcmp(Exp->children[1]->type_str, "LT") || !strcmp(Exp->children[1]->type_str, "LE")) {
			char* t1 = new_place();
			char* t2 = new_place();
			char* code1 = translate_Exp(Exp->children[0], t1);
			char* code2 = translate_Exp(Exp->children[2], t2);
			char relop[5];
			if (!strcmp(Exp->children[1]->type_str, "EQ")) {
				strcpy(relop, "==");
			} else if (!strcmp(Exp->children[1]->type_str, "NQ")) {
				strcpy(relop, "!=");
			} else if (!strcmp(Exp->children[1]->type_str, "GT")) {
				strcpy(relop, ">");
			} else if (!strcmp(Exp->children[1]->type_str, "GE")) {
				strcpy(relop, ">=");
			} else if (!strcmp(Exp->children[1]->type_str, "LT")) {
				strcpy(relop, "<");
			} else if (!strcmp(Exp->children[1]->type_str, "LE")) {
				strcpy(relop, "<=");
			}
			char* tac = (char*)malloc(strlen(code1) + strlen(code2) + 50);
			sprintf(tac, "%s%sIF %s %s %s GOTO %s\nGOTO %s\n", code1, code2, t1, relop, t2, lb_t, lb_f);
			return tac;
		}
	}
}

char* translate_Stmt(Node* Stmt) {
	if (Stmt->children_num == 1) { // CompSet
		if (!strcmp(Stmt->children[0]->type_str, "CompSt")) {
			return translate_Node(Stmt->children[0], 10);
		}
	} else if (Stmt->children_num == 2) {
		if (!strcmp(Stmt->children[0]->type_str, "Exp")) {
			return translate_Exp(Stmt->children[0], new_place());
		}
	} else if (Stmt->children_num == 3) { // RETURN Exp SEMI
		if (!strcmp(Stmt->children[0]->type_str, "RETURN")) {
			char* tp = new_place();
			char* code = translate_Exp(Stmt->children[1], tp);
			char* tac = (char*)malloc(strlen(code) + 20);
			sprintf(tac, "%sRETURN %s\n", code, tp);
			return tac;
		}
	} else if (Stmt->children_num == 5) {
		if (!strcmp(Stmt->children[0]->type_str, "IF")) {
			char* lb1 = new_label();
			char* lb2 = new_label();
			char* code1 = translate_cond_Exp(Stmt->children[2], lb1, lb2);
			char* code2 = translate_Stmt(Stmt->children[4]);
			char* tac = (char*)malloc(strlen(code1) + strlen(code2) + 50);
			sprintf(tac, "%sLABEL %s\n%sLABEL %s\n", code1, lb1, code2, lb2);
			return tac;
		} else if (!strcmp(Stmt->children[0]->type_str, "WHILE")) {
			char* lb1 = new_label();
			char* lb2 = new_label();
			char* lb3 = new_label();
			char* code1 = translate_cond_Exp(Stmt->children[2], lb2, lb3);
			char* code2 = translate_Stmt(Stmt->children[4]);
			char* tac = (char*)malloc(strlen(code1) + strlen(code2) + 50);
			sprintf(tac, "LABEL %s\n%sLABEL %s\n%sGOTO %s\nLABEL %s\n", lb1, code1, lb2, code2, lb1, lb3);
			return tac;
		}
	} else if (Stmt->children_num == 7) {
		if (!strcmp(Stmt->children[0]->type_str, "IF")) {
			char* lb1 = new_label();
			char* lb2 = new_label();
			char* lb3 = new_label();
			char* code1 = translate_cond_Exp(Stmt->children[2], lb1, lb2);
			char* code2 = translate_Stmt(Stmt->children[4]);
			char* code3 = translate_Stmt(Stmt->children[6]);
			char* tac = (char*)malloc(strlen(code1) + strlen(code2) + strlen(code3) + 50);
			sprintf(tac, "%sLABEL %s\n%sGOTO %s\nLABEL %s\n%sLABEL %s\n", code1, lb1, code2, lb3, lb2, code3, lb3);
			return tac;
		}
	}
}

char* translate_Dec(Node* Dec) {
	if (Dec->children_num == 3) {
		char* lv = Dec->children[0]->children[0]->value;
		char* tp = new_place();
		char* code1 = translate_Exp(Dec->children[2], tp);
		char* code2 = (char*)malloc(strlen(lv) + 10);
		sprintf(code2, "var%s := %s\n", lv, tp);
		char* tac = (char*)malloc(strlen(code1) + strlen(code2) + 10);
		sprintf(tac, "%s%s", code1, code2);
		return tac;
	}
}