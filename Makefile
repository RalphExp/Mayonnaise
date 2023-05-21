TARGET=cbc

CFLAGS = -g -I. -std=c++11

LDFLAGS =

$(TARGET): parser.o scanner.o cbc.o
	g++ $(FLAGS) $(LDFLAGS) -o$@ $^

scanner.hh parser.hh scanner.cc parser.cc: scanner.l parser.y
	flex scanner.l
	bison -d --color=always -ggraph -oparser.cc parser.y
#	bison -d --report=all --color=always -ggraph -oparser.cc parser.y

cbc.o: cbc.cc scanner.hh parser.hh
	g++ $(CFLAGS) -o$@ -c cbc.cc

parser.o: parser.cc parser.hh token.h
	g++ $(CFLAGS) -o$@ -c parser.cc
	
scanner.o: scanner.cc scanner.hh token.h
	g++ $(CFLAGS) -o$@ -c scanner.cc

clean:
	rm -rf *.hh
	rm -rf scanner.cc parser.cc
	rm -rf graph   
	rm -rf *.o
	rm -rf $(TARGET)
