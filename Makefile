TARGET=mayonnaise

mayonnaise: may.o main.o
	g++ -o$@ -lbison -lfl $<

may.c: may.y
	bison -d $< && mv may.tab.c may.c && mv may.tab.h may.h

may.o: may.c
	g++ -o$@ -c may.c

main.o: main.cc
	g++ -o$@ -c $^

clean:
	rm -rf may.h may.c
	rm -rf *.o
	rm -rf $(TARGET)
