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
int unique_counter;
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
        Type* read_function = (Type*)malloc(sizeof(Type));
        read_function->category = FUNCTION;
        Function* read_function_function = (Function*)malloc(sizeof(Function));
        read_function_function->returnType = get_primitive_type("int");
        read_function_function->args_num = 0;
        read_function->function = read_function_function;
        Type* write_function = (Type*)malloc(sizeof(Type));
        write_function->category = FUNCTION;
        Function* write_function_function = (Function*)malloc(sizeof(Function));
        write_function_function->returnType = get_primitive_type("int");
        write_function_function->args_num = 1;
        FieldList* to_write = (FieldList*)malloc(sizeof(FieldList));
        to_write->type = get_primitive_type("int");
        write_function_function->args = to_write;
        write_function->function = write_function_function;
        symtab_insert(function_symtab, "read", read_function);
        symtab_insert(function_symtab, "write", write_function);
        max_depth = 0;
        sp = 0;
        unique_counter = 0;
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
        traverse(root, 0); // for semantic checking
        char* code = translate_Node(root, 0);
        printf("%s", code);
        // printf("global variables:\n");
        // global_symtab = global_symtab->next;
        // while (global_symtab != NULL) {
        //     printf("%s: ", global_symtab->entry.key);
        //     print_type(global_symtab->entry.value, 1);
        //     global_symtab = global_symtab->next;
        //     if (global_symtab == NULL) {
        //         break;
        //     }
        // }                
        // printf("-----------\nfunctions:\n");
        // function_symtab = function_symtab->next;
        // while (function_symtab != NULL) {
        //     printf("function: ");
        //     print_type(function_symtab->entry.value, 1);
        //     function_symtab = function_symtab->next;
        //     if (function_symtab == NULL) {
        //         break;
        //     }
        //}       
        // printf("-----------\nstructures:\n");

        // structure_symtab = structure_symtab->next;
        // while (structure_symtab != NULL) {
        //     if (structure_symtab->entry.value == NULL) {
        //         break;
        //     }
        //     printf("%s: ", structure_symtab->entry.key);
        //     print_type(structure_symtab->entry.value, 1);
        //     structure_symtab = structure_symtab->next;
        //     if (structure_symtab == NULL) {
        //         break;
        //     }
        // }
        // printf("%d\n", max_depth);
        return EXIT_OK;
    }
    else{
        fputs("Too much arguments!\n", stderr);
        return EXIT_FAIL;
    }
}