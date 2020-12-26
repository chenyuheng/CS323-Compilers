splc:
	bison -d -v syntax.y -t
	flex lex.l
	gcc  syntax.tab.c node.c symtab.c main.c -lfl -o bin/splc -g
	rm lex.yy.c
	rm syntax.output
	rm syntax.tab.c
	rm syntax.tab.h
clean:
	rm lex.yy.c
	rm syntax.output
	rm syntax.tab.c
	rm syntax.tab.h
