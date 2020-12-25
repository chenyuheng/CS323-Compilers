%{
    #include"node.h"
    #include"symtab.h"
	#include"syntax.tab.h"

	extern int errorLexFlag;
	int yycolno = 1;
	#define YY_USER_ACTION \
		yylloc.first_line = yylineno;\
		yylloc.first_column = yycolno; \
        yylloc.last_line = yylineno;\
		yylloc.last_column = yycolno + yyleng; \
		yycolno += yyleng;

    void processToken(char* token) {
        yylval.type_node = get_node(token, yytext, yylineno, 0);
        if (yylval.type_node == NULL) {
            printf("Lexer: null result");
        }
       // printf("%s\n", token);
    }

    void processErrorA(char* cause) {
        errorLexFlag = 1;
        printf("Error type A at Line %d: %s %s\n", yylineno, cause, yytext);
    }
%}
letter [a-zA-Z_]
digit [0-9]
nonzeroDigit [1-9]
hexdigit [0-9a-fA-F]
nonzeroHexdigit [1-9a-fA-F]
int {nonzeroDigit}{digit}*|"0" 
hexint ("0x"|"0X")({nonzeroHexdigit}{hexdigit}*|0)
float {int}+\.{int}+
char \'.\'
hexchar \'\\[xX]{hexdigit}{1,2}\'
id {letter}({letter}|{digit})*
nonid {digit}+{id}
nondigit 0+{digit}*
ws [ \t]
br [\n\r]
nonhexchar \'\\[xX][0-9a-zA-Z]*\'


%option yylineno

%%

{int}       { processToken("INT");      return INT; }
{float}     { processToken("FLOAT");    return FLOAT; }
{char}      { processToken("CHAR");     return CHAR; }
{hexint}    { 
    int num = strtol(yytext, NULL, 0);
    sprintf(yytext, "%d", num);
    processToken("INT");
    return INT; 
}
{hexchar}   {
    yytext = yytext + 1;
    yytext[strlen(yytext) - 1] = '\0';
    processToken("CHAR");
    return CHAR;
}

"int"|"float"|"char"    { processToken("TYPE");     return TYPE; }
"struct"                { processToken("STRUCT");   return STRUCT; }
"if"                    { processToken("IF");       return IF; }
"else"                  { processToken("ELSE");     return ELSE; }
"while"                 { processToken("WHILE");    return WHILE; }
"for"                   { processToken("FOR");      return FOR; }
"return"                { processToken("RETURN");   return RETURN; }

{ws} { /* whitespace */ }
{br} { yycolno = 1; }
"//"    { 
    char c;
    while((c=input()) != '\n') {
        if (c == 0) {
            processErrorA("unfinished comment");
            break;
        }
    };
}
"/*"    {
    char c1, c2;
    c1 = input();
    c2 = input();
    while (c1 != '*' || c2 != '/') {
        c1 = c2;
        c2 = input();
        if (c2 == 0) {
            processErrorA("unclosed comment");
            break;
        }
    }
}

{id} { processToken("ID");  return ID; }

"."     { processToken("DOT");      return DOT; }
";"     { processToken("SEMI");     return SEMI; }
","     { processToken("COMMA");    return COMMA; }
"="     { processToken("ASSIGN");   return ASSIGN; }
"<"     { processToken("LT");       return LT; }
"<="    { processToken("LE");       return LE; }
">"     { processToken("GT");       return GT; }
">="    { processToken("GE");       return GE; }
"!="    { processToken("NE");       return NE; }
"=="    { processToken("EQ");       return EQ; }
"+"     { processToken("PLUS");     return PLUS; }
"-"     { processToken("MINUS");    return MINUS; }
"*"     { processToken("MUL");      return MUL; }
"/"     { processToken("DIV");      return DIV; }
"&&"    { processToken("AND");      return AND; }
"||"    { processToken("OR");       return OR;  }
"!"     { processToken("NOT");      return NOT; }
"("     { processToken("LP");       return LP; }
")"     { processToken("RP");       return RP; }
"["     { processToken("LB");       return LB; }
"]"     { processToken("RB");       return RB; }
"{"     { processToken("LC");       return LC; }
"}"     { processToken("RC");       return RC; }

{nonid}         { processErrorA("unknown lexeme"); return ID; }
{nondigit}      { processErrorA("unknown lexeme"); return SPECIAL; }
{nonhexchar}    { processErrorA("unknown lexeme"); return SPECIAL; }
.               { processErrorA("unknown lexeme"); return SPECIAL; }
%%

