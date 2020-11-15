%{
	#include"lex.yy.c"
    extern void yyerror(char*);
    extern int yylineno;
    extern int errorSyntaxFlag;
    extern Node* root;
    extern symtab* global_symtab;

    void processErrorB(char* cause, int lineno) {
        errorSyntaxFlag = 1;
        printf("Error type B at Line %d: %s\n", lineno, cause);
    }
%}
%union {
    int type_int;
    float type_float;
    char type_char;
    double type_double;
    Node* type_node;
}

%token <type_node> INT FLOAT CHAR
%token <type_node> ID
%token <type_node> SEMI COMMA ASSIGN
%token <type_node> LT LE GT GE NE EQ
%token <type_node> PLUS MINUS MUL DIV
%token <type_node> AND OR NOT
%token <type_node> DOT
%token <type_node> TYPE
%token <type_node> LP RP LB RB LC RC
%token <type_node> STRUCT RETURN IF ELSE WHILE FOR
%token <type_node> SPECIAL

/* declared non-terminals */
/* high-level definition */
%type <type_node> Program ExtDefList ExtDef ExtDecList
/* Specifiers */
%type <type_node> Specifier StructSpecifier 
/* Declarators */
%type <type_node> VarDec FunDec VarList ParamDec
/* Statements */
%type <type_node> CompSt StmtList Stmt
/* Local Definitions */
%type <type_node> DefList Def DecList Dec 
/* Expressions */
%type <type_node> Exp Args ExpOrNull /* for FOR */

%nonassoc LOWER_THAN_ASSIGN
%right ASSIGN
%left OR
%left AND
%left LT LE GT GE NE EQ  
%left MINUS PLUS
%left MUL DIV
%right NOT
%left DOT
%left LB RB
%left LP RP 

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%%

/* high-level definition */
Program: ExtDefList {
    $$ = get_node("Program", "", @$.first_line, 1, $1);
    root = $$;
};
ExtDefList: ExtDef ExtDefList {
    $$ = get_node("ExtDefList", "", @$.first_line, 2, $1, $2);
} | /* empty */ {
    $$ = NULL;
};
ExtDef: Specifier ExtDecList SEMI {
    $$ = get_node("ExtDef", "", @$.first_line, 3, $1, $2, $3);
    Type* base_type = $1->type;
} | Specifier SEMI {
    $$ = get_node("ExtDef", "", @$.first_line, 2, $1, $2);
} | Specifier ExtDecList error {
    processErrorB("Missing semicolon ';'", @$.first_line);
} | Specifier error {
    processErrorB("Missing semicolon ';'", @$.first_line);
} | Specifier FunDec CompSt {
    $$ = get_node("ExtDef", "", @$.first_line, 3, $1, $2, $3);
};
ExtDecList: VarDec {
    $$ = get_node("ExtDecList", "", @$.first_line, 1, $1);
} | VarDec COMMA ExtDecList {
    $$ = get_node("ExtDecList", "", @$.first_line, 3, $1, $2, $3);
};

/* specifier */
Specifier: TYPE {
    $$ = get_node("Specifier", "", @$.first_line, 1, $1);
    $$->type = $1->type;
} | StructSpecifier {
    $$ = get_node("Specifier", "", @$.first_line, 1, $1);
};
StructSpecifier: STRUCT ID LC DefList RC {
    $$ = get_node("StructSpecifier", "", @$.first_line, 5, $1, $2, $3, $4, $5);
} | STRUCT ID LC DefList error {
    processErrorB("Missing closing curly brace '}'", @$.first_line);
} | STRUCT ID {
    $$ = get_node("StructSpecifier", "", @$.first_line, 2, $1, $2);
};

/* declarator */
VarDec: ID {
    $$ = get_node("VarDec", "", @$.first_line, 1, $1);
} | VarDec LB INT RB {
    $$ = get_node("VarDec", "", @$.first_line, 4, $1, $2, $3, $4);
} | VarDec LB INT error {
    processErrorB("Missing closing bracket ']'", @$.first_line);
};
FunDec: ID LP VarList RP {
    $$ = get_node("FunDec", "", @$.first_line, 4, $1, $2, $3, $4);
} | ID LP VarList error {
    processErrorB("Missing closing parenthesis ')'", @$.first_line);
} | ID LP RP {
    $$ = get_node("FunDec", "", @$.first_line, 3, $1, $2, $3);
} | ID LP error {
    processErrorB("Missing closing parenthesis ')'", @$.first_line);
};
VarList: ParamDec COMMA VarList {
    $$ = get_node("VarList", "", @$.first_line, 3, $1, $2, $3);
} | ParamDec {
    $$ = get_node("VarList", "", @$.first_line, 1, $1);
};
ParamDec: Specifier VarDec {
    $$ = get_node("ParamDec", "", @$.first_line, 2, $1, $2);
};

/* statement */
CompSt: LC DefList StmtList RC {
    $$ = get_node("CompSt", "", @$.first_line, 4, $1, $2, $3, $4);
} | LC DefList StmtList DefList error RC {
    processErrorB("Missing specifier", @3.first_line);
} | LC DefList StmtList error {
    processErrorB("Missing closing curly brace '}'", @$.first_line);
} | LC RC {
    processErrorB("Missing statements in composition set.", @$.first_line);
};
StmtList: Stmt StmtList {
    $$ = get_node("StmtList", "", @$.first_line, 2, $1, $2);
} | /* empty */ {
    $$ = NULL;
};
Stmt: Exp SEMI {
    $$ = get_node("Stmt", "", @$.first_line, 2, $1, $2);
} | CompSt {
    $$ = get_node("Stmt", "", @$.first_line, 1, $1);
} | RETURN Exp SEMI {
    $$ = get_node("Stmt", "", @$.first_line, 3, $1, $2, $3);
} | Exp error {
    processErrorB("Missing semicolon ';'", @$.first_line);
} | RETURN Exp error {
    processErrorB("Missing semicolon ';'", @$.first_line);
} | IF LP Exp RP Stmt {
    $$ = get_node("Stmt", "", @$.first_line, 5, $1, $2, $3, $4, $5);
} | IF LP Exp RP Stmt ELSE Stmt {
    $$ = get_node("Stmt", "", @$.first_line, 7, $1, $2, $3, $4, $5, $6, $7);
} | WHILE LP Exp RP Stmt {
    $$ = get_node("Stmt", "", @$.first_line, 5, $1, $2, $3, $4, $5);
} | FOR LP ExpOrNull SEMI ExpOrNull SEMI ExpOrNull RP Stmt {
    $$ = get_node("Stmt", "", @$.first_line, 9, $1, $2, $3, $4, $5, $6, $7, $8, $9);
} | IF LP Exp error Stmt {
    processErrorB("Missing closing parenthesis ')'", @$.first_line);
} | IF LP Exp error Stmt ELSE Stmt {
    processErrorB("Missing closing parenthesis ')'", @$.first_line);
} | IF LP RP error {
    processErrorB("Missing exp in IF statement", @$.first_line);
    processErrorB("Missing statement in IF statement", @$.first_line);
} | IF LP RP error Stmt ELSE {
    processErrorB("Missing exp in IF statement", @$.first_line);
    processErrorB("Missing statement in ELSE statement", @6.first_line);
} | IF LP RP error Stmt ELSE Stmt {
    processErrorB("Missing exp in IF statement", @$.first_line);
} | WHILE LP Exp error Stmt {
    processErrorB("Missing closing parenthesis ')'", @$.first_line);
} | FOR LP ExpOrNull SEMI ExpOrNull SEMI ExpOrNull error Stmt {
    processErrorB("Missing closing parenthesis ')'", @$.first_line);
} | FOR LP ExpOrNull SEMI ExpOrNull error RP Stmt {
    processErrorB("Missing semicolon ';'", @$.first_line);
} | FOR LP ExpOrNull error RP Stmt {
    processErrorB("Missing semicolon ';'", @$.first_line);
};
;

/* local definition */
DefList: Def DefList {
    $$ = get_node("DefList", "", @$.first_line, 2, $1, $2);
} | /* empty */ {
    $$ = NULL;
};
Def: Specifier DecList SEMI {
    $$ = get_node("Def", "", @$.first_line, 3, $1, $2, $3);
} | Specifier DecList error {
    processErrorB("Missing semicolon ';'", @$.first_line);
};
DecList: Dec {
    $$ = get_node("DecList", "", @$.first_line, 1, $1);
} | Dec COMMA DecList {
    $$ = get_node("DecList", "", @$.first_line, 3, $1, $2, $3);
};
Dec: VarDec {
    $$ = get_node("Dec", "", @$.first_line, 1, $1);
} | VarDec ASSIGN Exp {
    $$ = get_node("Dec", "", @$.first_line, 3, $1, $2, $3);
}

/* Expression */
Exp: Exp ASSIGN Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp AND Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp OR Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp LT Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp LE Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp GT Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp GE Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp NE Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp EQ Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp PLUS Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp MINUS Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp MUL Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | Exp DIV Exp {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | LP Exp RP {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | LP Exp error {
    processErrorB("Missing closing parenthesis ')'", @$.first_line);
} | MINUS Exp {
    $$ = get_node("Exp", "", @$.first_line, 2, $1, $2);
} | NOT Exp {
    $$ = get_node("Exp", "", @$.first_line, 2, $1, $2);
} | ID LP Args RP {
    $$ = get_node("Exp", "", @$.first_line, 4, $1, $2, $3, $4);
} | ID LP Args error {
    processErrorB("Missing closing parenthesis ')'", @$.first_line);
} | ID LP RP {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | ID LP error {
    processErrorB("Missing closing parenthesis ')'", @$.first_line);
} | Exp LB Exp RB {
    $$ = get_node("Exp", "", @$.first_line, 4, $1, $2, $3, $4);
} | Exp LB Exp error {
    processErrorB("Missing closing bracket ']'", @$.first_line);
} | Exp DOT ID {
    $$ = get_node("Exp", "", @$.first_line, 3, $1, $2, $3);
} | ID {
    $$ = get_node("Exp", "", @$.first_line, 1, $1);
} | INT {
    $$ = get_node("Exp", "", @$.first_line, 1, $1);
} | FLOAT {
    $$ = get_node("Exp", "", @$.first_line, 1, $1);
} | CHAR {
    $$ = get_node("Exp", "", @$.first_line, 1, $1);
} | SPECIAL %prec LOWER_THAN_ASSIGN { 
    $$ = NULL;
} | Exp SPECIAL Exp %prec LOWER_THAN_ASSIGN {
    $$ = NULL;
};

Args: Exp COMMA Args {
    $$ = get_node("Args", "", @$.first_line, 3, $1, $2, $3);
} | Exp {
    $$ = get_node("Args", "", @$.first_line, 1, $1);
};

ExpOrNull: Exp {
    $$ = get_node("ExpOrNull", "", @$.first_line, 1, $1);
} | /* empty */ {
    $$ = NULL;
}
%%

void yyerror(char* msg) {
    if (errorSyntaxFlag == 0) {
	    fprintf(stderr, "Error type B at Line %d:  %s\n", yylineno, msg);
    }
}
