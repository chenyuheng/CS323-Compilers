%{
    #include"lex.yy.c"
    int result;
    void yyerror(const char *s){
        result = 0;
    }
%}
%token LP RP LB RB LC RC
%%
quiz: expr {result = 1;}

expr: LP RP
      | LB RB
      | LC RC
      | LP expr RP
      | LB expr RB
      | LC expr RC
      | expr expr
      ;
%%

int validParentheses(char *expr){
    result = 0;
    yy_scan_string(expr);
    yyparse();
    return result;
}

int main() {
    int r = validParentheses("(]");
    printf("%d\n", r);
    return result;
}
