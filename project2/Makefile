splc:
	bison -d -v syntax.y -t
	flex lex.l
	gcc  syntax.tab.c node.c main.c -lfl -o bin/splc
	rm lex.yy.c
	rm syntax.output
	rm syntax.tab.c
	rm syntax.tab.h
clean:
	rm -r test_out