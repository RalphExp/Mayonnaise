TARGET=mayonnaise

CFLAGS = -g -I. -std=c++11

LDFLAGS = -lfl

cbc: parser.o scanner.o cbc.o
	g++ $(CXXFLAGS) $(LDFLAGS) -o$@ $^

parser.hh parser.cc scanner.cc scanner.h: scanner.l parser.y
	flex scanner.l
	bison -d -oparser.cc parser.y

parser.o: parser.cc parser.hh
	g++ $(CFLAGS) -o$@ -c parser.cc
	
scanner.o: scanner.cc scanner.h
	g++ $(CFLAGS) -o$@ -c scanner.cc

cbc.o: cbc.cc 
	g++ $(CFLAGS) -o$@ -c cbc.cc

clean:
	rm -rf *.hh scanner.h
	rm -rf scanner.cc parser.cc
	rm -rf *.o
	rm -rf $(TARGET)
