%{
    #include"lex.yy.c"
    void yyerror(const char*);
    JsonObject *root;
     
int sp = 0;
unsigned int symbol_table[100000];

void create_table() {
    symbol_table[++sp] = 0;
}
int insert_key(char c) {
    int v = c - 'a';
    unsigned int p = 1 << v;
    if ((symbol_table[sp] & p) != 0) {
        return -1;
    }
    symbol_table[sp] |= p;
    return 0;
}
void delete_table() {
    sp--;
}
%}
%union {
  JsonObject* type_obj;
  ObjectMember* type_mem;
  ArrayValue* type_arr;
}
%token LC RC LB RB COLON COMMA
%token <type_obj> STRING NUMBER TRUE FALSE VNULL
%token LZN
%type <type_obj> Json Value;
%type <type_mem> Object Member Members;
%type <type_arr> Array Values;
%%

Json:
      Value { $$ = $1; root = $1;}
    ;
Value:
      Object {
        JsonObject* node = (JsonObject*)malloc(sizeof(JsonObject));
        node->category = OBJ;
        node->members = $1;
        $$ = node;
      }
    | Array {
        JsonObject* node = (JsonObject*)malloc(sizeof(JsonObject));
        node->category = ARR;
        node->values = $1;
        $$ = node;
      }
    | STRING { $$ = $1; }
    | NUMBER { $$ = $1; }
    | LZN error { puts("cannot have leading zero, recovered"); }
    | TRUE { $$ = $1; }
    | FALSE { $$ = $1; }
    | VNULL { $$ = $1; }
    ;
Object:
      LC RC { $$ = NULL; }
    | LC { create_table(); } Members RC { $$ = $3; delete_table(); }
    ;
Members:
      Member { $$ = $1; }
    | Member COMMA Members {
      $1->next = $3;
      $$ = $1;
    }
    ;
Member:
      STRING COLON Value {
        ObjectMember* node = (ObjectMember*)malloc(sizeof(ObjectMember));
        node->key = $1->string;
        node->value = $3;
        node->next = NULL;
        $$ = node;
        if (insert_key(node->key[1])) {
            printf("duplicate key: \"%c\"\n", node->key[1]);
        }
      }
    ;
Array:
      LB RB { $$ = NULL; }
    | LB Values RB { $$ = $2; }
    ;
Values:
      Value {
        ArrayValue* node = (ArrayValue*)malloc(sizeof(ArrayValue));
        node->value = $1;
        node->next = NULL;
        $$ = node;
      }
    | Value COMMA Values {
        ArrayValue* node = (ArrayValue*)malloc(sizeof(ArrayValue));
        node->value = $1;
        node->next = $3;
        $$ = node;
    }
    ;
%%
void printArrayValue(struct ArrayValue *arval, int ind, int is_head);
void printObjectMember(struct ObjectMember *member, int ind, int is_head);
void printJsonObject(struct JsonObject *node, int ind) {
  if (node == NULL) {
    return;
  }
  switch (node->category) {
    case OBJ:
      printObjectMember(node->members, ind, 1);
      break;
    case ARR:
      printArrayValue(node->values, ind, 1);
      break;
    case STR:
      printf("%*s", ind * 2, "");
      printf("%s\n", node->string);
      break;
    case NUM:
      printf("%*s", ind * 2, "");
      printf("%lf\n", node->number);
      break;
    case BOO:
      printf("%*s", ind * 2, "");
      if (node->boolean) {
        printf("True\n");
      } else {
        printf("False\n");
      }
      break;
    case VNU:
      printf("%*s", ind * 2, "");
      printf("null\n");
      break;
    default:
      break;
  }
}

void printArrayValue(struct ArrayValue *arval, int ind, int is_head) {
  if (is_head) {
    printf("%*s", ind * 2, "");
    printf("[ \n");
  }
  printJsonObject(arval->value, ind + 1);
  if (arval->next) {
    printArrayValue(arval->next, ind, 0);
  } else {
    printf("%*s", ind * 2, "");
    printf("]\n");
  }
}

void printObjectMember(struct ObjectMember *member, int ind, int is_head) {
  if (is_head) {
    printf("%*s", ind * 2, "");
    printf("{\n");
  }
    printf("%*s", ind * 2, "");
  printf("%s: \n", member->key);
  printJsonObject(member->value, ind + 1);
  if (member->next) {
    printObjectMember(member->next, ind, 0);
  } else {
    printf("%*s", ind * 2, "");
    printf("}\n");
  }
}



void yyerror(const char *s){
    printf("syntax error.\n");
}


int main(int argc, char **argv){
    symbol_table[0] = 0;
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        exit(-1);
    }
    else if(!(yyin = fopen(argv[1], "r"))) {
        perror(argv[1]);
        exit(-1);
    }
    yyparse();
   // printJsonObject(root, 0);
    return 0;
}
