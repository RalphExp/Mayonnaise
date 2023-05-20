TARGET=cbc

CFLAGS = -g -I. -std=c++11

LDFLAGS =

$(TARGET): parser.o scanner.o cbc.o
	g++ $(FLAGS) $(LDFLAGS) -o$@ $^

scanner.hh parser.hh scanner.cc parser.cc: scanner.l parser.y
	flex scanner.l
	bison -d --color=always -ggraph -oparser.cc parser.y
# bison -d -Wcounterexamples --color=always -ggraph -oparser.cc parser.y

parser.o: parser.cc parser.hh
	g++ $(CFLAGS) -o$@ -c parser.cc
	
scanner.o: scanner.cc scanner.hh
	g++ $(CFLAGS) -o$@ -c scanner.cc

cbc.o: cbc.cc scanner.hh parser.hh
	g++ $(CFLAGS) -o$@ -c cbc.cc

clean:
	rm -rf *.hh
	rm -rf scanner.cc parser.cc
	rm -rf graph   
	rm -rf *.o
	rm -rf $(TARGET)
