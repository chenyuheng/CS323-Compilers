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
test_all:
	mkdir test_out
	./bin/splc test/test_1_r01.spl > test_out/test_1_r01.out
	./bin/splc test/test_1_r02.spl > test_out/test_1_r02.out
	./bin/splc test/test_1_r03.spl > test_out/test_1_r03.out
	./bin/splc test/test_1_r04.spl > test_out/test_1_r04.out
	./bin/splc test/test_1_r05.spl > test_out/test_1_r05.out
	./bin/splc test/test_1_r06.spl > test_out/test_1_r06.out
	./bin/splc test/test_1_r07.spl > test_out/test_1_r07.out
	./bin/splc test/test_1_r08.spl > test_out/test_1_r08.out
	./bin/splc test/test_1_r09.spl > test_out/test_1_r09.out
	./bin/splc test/test_1_r10.spl > test_out/test_1_r10.out
	./bin/splc test/test_1_r11.spl > test_out/test_1_r11.out
	./bin/splc test/test_1_r12.spl > test_out/test_1_r12.out
diff1:
	diff test/test_1_r01.out test_out/test_1_r01.out 
diff2:
	diff test/test_1_r02.out test_out/test_1_r02.out 
diff3:
	diff test/test_1_r03.out test_out/test_1_r03.out 
diff4:
	diff test/test_1_r04.out test_out/test_1_r04.out 
diff5:
	diff test/test_1_r05.out test_out/test_1_r05.out 
diff6:
	diff test/test_1_r06.out test_out/test_1_r06.out 
diff7:
	diff test/test_1_r07.out test_out/test_1_r07.out 
diff8:
	diff test/test_1_r08.out test_out/test_1_r08.out 
diff9:
	diff test/test_1_r09.out test_out/test_1_r09.out 
diff10:
	diff test/test_1_r10.out test_out/test_1_r10.out 
diff11:
	diff test/test_1_r11.out test_out/test_1_r11.out 
diff12:
	diff test/test_1_r12.out test_out/test_1_r12.out
diff:
	diff test/test_1_r01.out test_out/test_1_r01.out 
	diff test/test_1_r02.out test_out/test_1_r02.out 
	diff test/test_1_r03.out test_out/test_1_r03.out 
	diff test/test_1_r04.out test_out/test_1_r04.out 
	diff test/test_1_r05.out test_out/test_1_r05.out 
	diff test/test_1_r06.out test_out/test_1_r06.out 
	diff test/test_1_r07.out test_out/test_1_r07.out 
	diff test/test_1_r08.out test_out/test_1_r08.out 
	diff test/test_1_r09.out test_out/test_1_r09.out 
	diff test/test_1_r10.out test_out/test_1_r10.out 
	diff test/test_1_r11.out test_out/test_1_r11.out 
	diff test/test_1_r12.out test_out/test_1_r12.out


