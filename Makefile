TARGET=mayonnaise

CFLAGS = -g -I.

LDFLAGS =

mayonnaise: may.o scan.o main.o
	g++ $(CXXFLAGS) -o$@ $^

scan.cc: may.l
	flex -oscan.c $< && mv scan.c scan.cc

may.h may.cc: may.y
	bison -d -omay.c $< && mv may.c may.cc

may.o: may.h may.cc
	g++ $(CFLAGS) -o$@ -c may.cc
	
scan.o: may.h scan.cc
	g++ $(CFLAGS) -o$@ -c scan.cc

main.o: may.h main.cc 
	g++ $(CFLAGS) -o$@ -c main.cc

clean:
	rm -rf may.h may.hh may.c may.cc scan.c scan.cc
	rm -rf *.o
	rm -rf $(TARGET)
