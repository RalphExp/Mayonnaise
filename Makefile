TARGET=mayonnaise

mayonnaise: main.o scan.o may.o
	g++ -o$@ $<

scan.c: may.l
	flex -o$@ $<

may.c: may.y
	bison -d -o$@ $<

may.o: may.c
	g++ -o$@ -c may.c

main.o: main.cc
	g++ -o$@ -c $^

clean:
	rm -rf may.h may.c
	rm -rf *.o
	rm -rf $(TARGET)
