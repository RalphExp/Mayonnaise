TARGET=cbc

CFLAGS = -g -I. -std=c++11

LDFLAGS =

$(TARGET): parser.o scanner.o cbc.o
	g++ $(CXXFLAGS) $(LDFLAGS) -o$@ $^

parser.hh parser.cc scanner.cc scanner.h: scanner.l parser.y
	flex scanner.l
	bison -d -Wcounterexamples --color=always -ggraph -oparser.cc parser.y
# bison -d -Wcounterexamples -Wno-conflicts-sr --color=always -ggraph -oparser.cc parser.y

parser.o: parser.cc parser.hh
	g++ $(CFLAGS) -o$@ -c parser.cc
	
scanner.o: scanner.cc scanner.h
	g++ $(CFLAGS) -o$@ -c scanner.cc

cbc.o: cbc.cc scanner.h parser.hh
	g++ $(CFLAGS) -o$@ -c cbc.cc

clean:
	rm -rf scanner.h parser.hh
	rm -rf scanner.cc parser.cc
	rm -rf graph   
	rm -rf *.o
	rm -rf $(TARGET)
