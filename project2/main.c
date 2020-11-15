#include "node.h"
#include "symtab.h"
#include <stdlib.h>
#include <stdio.h>
#include "syntax.tab.h"

    
#define EXIT_OK 0
#define EXIT_FAIL 1
int errorLexFlag;
int errorSyntaxFlag;
extern int yyrestart(FILE *f);
Node *root;
symtab* global_symtab;
extern int yylineno;

int main(int argc, char **argv){
    char *file_path;
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return EXIT_FAIL;
    }
    else if(argc == 2){
        global_symtab = symtab_init();
        FILE *f = fopen(argv[1], "r");
        if(!f){
            perror(argv[1]);
            return 1;
        }
        root = NULL;
        yylineno = 1;
        yyrestart(f);
        yyparse();
	    if(errorLexFlag == 0 && errorSyntaxFlag == 0) {
            print_tree(root, 0);
        }
        return EXIT_OK;
    }
    else{
        fputs("Too much arguments!\n", stderr);
        return EXIT_FAIL;
    }
}