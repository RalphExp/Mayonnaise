TARGET=mayonnaise

CFLAGS = -I.

LDFLAGS =

mayonnaise: may.o scan.o main.o
	g++ $(CXXFLAGS) -o$@ $^

scan.c: may.l
	flex -o$@ $<

may.h may.c: may.y
	bison -d -omay.c $<

may.o: may.h may.c
	gcc $(CFLAGS) -o$@ -c may.c
	
scan.o: may.h scan.c
	gcc $(CFLAGS) -o$@ -c scan.c

main.o: may.h main.cc 
	g++ $(CFLAGS) -o$@ -c main.cc

clean:
	rm -rf may.h may.hh may.c may.cc scan.c scan.cc
	rm -rf *.o
	rm -rf $(TARGET)
