TARGET=cbc

CFLAGS = -g -I. -Iinclude -std=c++11

LDFLAGS =

$(TARGET): parser.o scanner.o node.o util.o cbc.o
	g++ $(FLAGS) $(LDFLAGS) -o$@ $^

scanner.cc parser.cc: scanner.l parser.y
	flex scanner.l
	bison -d --color=always -ggraph -oparser.cc parser.y
#	bison -d --report=all --color=always -ggraph -oparser.cc parser.y

%.o: %.cc
	g++ $(CFLAGS) -o$@ -c $<

clean:
	rm -rf *.hh
	rm -rf scanner.cc parser.cc
	rm -rf graph   
	rm -rf *.o
	rm -rf $(TARGET)
