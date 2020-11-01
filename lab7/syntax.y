%{
    #include"lex.yy.c"
    void yyerror(const char*);
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
%type <type_mem> Member Members Object;
%type <type_arr> Array Values;
%%

Json:
      Value { $$ = $1; }
    ;
Value:
      Object {
        JsonObject* node = (JsonObject*)malloc(sizeof(JsonObject));
        node->category = OBJ;
        node->members = $1;
        $$ = node;
      }
    | Object Value error { puts("misplaced quoted value, recovered"); }
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
    | LC Members RC { $$ = $2; }
    ;
Members:
      Member { $$ = $1; }
    | Member COMMA Members {
      $1->next = $3;
      $$ = $1;
    }
    | Member COLON Members error { puts("colon instead of comma, recovered"); }
    ;
Member:
      STRING COLON Value {
        ObjectMember* node = (ObjectMember*)malloc(sizeof(ObjectMember));
        node->key = $1->string;
        node->value = $3;
        node->next = NULL;
        $$ = node;
      }
      | STRING Value error { puts("missing colon, recovered"); }
      | STRING COMMA Value error { puts("comma instead of colon, recovered"); }
      | STRING COLON Value COMMA error { puts("extra comma, recovered"); }
      | STRING COLON COLON Value error { puts("double colon, recovered"); }
    ;
Array:
      LB RB { $$ = NULL; }
    | LB Values RB { $$ = $2; }
    | LB Values RC error { puts("unmatched right bracket, recovered"); }
    | LB Values RB COMMA error { puts("comma after the close, recovered"); }
    | LB Values COMMA COMMA RB error { puts("double extra comma, recovered"); }
    | LB Values error { puts("unclosed array, recovered"); }
    | LB Values RB RB error { puts("extra close, recovered"); }
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
    | Value COMMA error { puts("extra comma, recovered"); }
    | COMMA Values error { puts("missing value, recovered"); }
    ;
%%

void yyerror(const char *s){
    printf("syntax error: ");
}

int main(int argc, char **argv){
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        exit(-1);
    }
    else if(!(yyin = fopen(argv[1], "r"))) {
        perror(argv[1]);
        exit(-1);
    }
    yyparse();
    return 0;
}
