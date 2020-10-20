%{
    #include"lex.yy.c"
    void yyerror(const char*);
%}

%token LC RC LB RB COLON COMMA
%token STRING NUMBER LZN
%token TRUE FALSE VNULL
%%

Json:
      Value
    ;
Value:
      Object
    | Object Value error { puts("misplaced quoted value, recovered"); }
    | Array
    | STRING
    | NUMBER
    | LZN error { puts("cannot have leading zero, recovered"); }
    | TRUE
    | FALSE
    | VNULL
    ;
Object:
      LC RC
    | LC Members RC
    ;
Members:
      Member
    | Member COMMA Members
    | Member COLON Members error { puts("colon instead of comma, recovered"); }
    ;
Member:
      STRING COLON Value
      | STRING Value error { puts("missing colon, recovered"); }
      | STRING COMMA Value error { puts("comma instead of colon, recovered"); }
      | STRING COLON Value COMMA error { puts("extra comma, recovered"); }
      | STRING COLON COLON Value error { puts("double colon, recovered"); }
    ;
Array:
      LB RB
    | LB Values RB
    | LB Values RC error { puts("unmatched right bracket, recovered"); }
    | LB Values RB COMMA error { puts("comma after the close, recovered"); }
    | LB Values COMMA COMMA RB error { puts("double extra comma, recovered"); }
    | LB Values error { puts("unclosed array, recovered"); }
    | LB Values RB RB error { puts("extra close, recovered"); }
    ;
Values:
      Value
    | Value COMMA Values
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
