%{
    #include"json_def.h"
    #include"syntax.tab.h"
    #include <stdlib.h>

    void processToken(int type) {
        JsonObject* node = (JsonObject*)malloc(sizeof(JsonObject));
        switch(type) {
            case 0:
                node->category = STR;
                node->string = strdup(yytext);
                break;
            case 1:
                node->category = NUM;
                node->number = strtod(yytext, NULL);
                break;
            case 2:
                node->category = BOO;
                node->boolean = 0;
                break;
            case 3:
                node->category = BOO;
                node->boolean = 1;
                break;
            case 4:
                node->category = VNU;
                break;
        }
        yylval.type_obj = node;
    }
%}
%option noyywrap

unic u[0-9a-fA-F]{4}
esc \\([\"\\/bfnrt]|{unic})
scp [^"\\\x00-\x1f]
string \"({esc}|{scp})*\"

int 0|[1-9][0-9]*
frac \.[0-9]+
exp [Ee][+-]?[0-9]+
number -?{int}{frac}?{exp}?
lzeronumber -?0{int}{frac}?{exp}?

empty [ \n\r\t]

%%

"{" { return LC; }
"}" { return RC; }
"[" { return LB; }
"]" { return RB; }
":" { return COLON; }
"," { return COMMA; }

"true" { processToken(3); return TRUE; }
"false" { processToken(2); return FALSE; }
"null" { processToken(4); return VNULL; }

{string} { processToken(0); return STRING; }
{number} { processToken(1); return NUMBER; }
{lzeronumber} { return LZN;}
{empty} {}

. { printf("lexical error: %s\n", yytext); }
