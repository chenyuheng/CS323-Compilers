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
symtab* function_symtab;
symtab* structure_symtab;
symtab** symtab_stack;
int max_depth;
int sp;
extern int yylineno;

int main(int argc, char **argv){
    char *file_path;
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return EXIT_FAIL;
    }
    else if(argc == 2){
        global_symtab = symtab_init();
        function_symtab = symtab_init();
        structure_symtab = symtab_init();
        max_depth = 0;
        sp = 0;
        FILE *f = fopen(argv[1], "r");
        if(!f){
            perror(argv[1]);
            return 1;
        }
        root = NULL;
        yylineno = 1;
        yyrestart(f);
        yyparse();
        completeSymbolTable();
        symtab_stack = (symtab**)malloc(sizeof(symtab*) * 1000);
        symtab_stack[0] = global_symtab;
        printf("global variables:\n");
        global_symtab = global_symtab->next;
        while (global_symtab != NULL) {
            printf("%s: ", global_symtab->entry.key);
            print_type(global_symtab->entry.value, 1);
            global_symtab = global_symtab->next;
            if (global_symtab == NULL) {
                break;
            }
        }                
        printf("-----------\nfunctions:\n");
        function_symtab = function_symtab->next;
        while (function_symtab != NULL) {
            printf("function: ");
            print_type(function_symtab->entry.value, 1);
            function_symtab = function_symtab->next;
            if (function_symtab == NULL) {
                break;
            }
        }       
        printf("-----------\nstructures:\n");

        structure_symtab = structure_symtab->next;
        while (structure_symtab != NULL) {
            if (structure_symtab->entry.value == NULL) {
                break;
            }
            printf("%s: ", structure_symtab->entry.key);
            print_type(structure_symtab->entry.value, 1);
            structure_symtab = structure_symtab->next;
            if (structure_symtab == NULL) {
                break;
            }
        }
        printf("%d\n", max_depth);
        return EXIT_OK;
    }
    else{
        fputs("Too much arguments!\n", stderr);
        return EXIT_FAIL;
    }
}