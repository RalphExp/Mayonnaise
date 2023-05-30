TARGET=may

CFLAGS = -g -I. -Iinclude -std=c++11

LDFLAGS =

AST_OBJ = $(patsubst %.cc, %.o, $(wildcard ast/*.cc))

PARSER_OBJ = \
	$(patsubst %.cc, %.o, $(wildcard parser/*.cc)) \
    $(patsubst %.l, %.o, $(wildcard parser/*.l)) \
    $(patsubst %.y, %.o, $(wildcard parser/*.y))

UTIL_OBJ = $(patsubst %.cc, %.o, $(wildcard util/*.cc))

COMPILER_OBJ = $(patsubst %.cc, %.o, $(wildcard *.cc))

$(TARGET): $(UTIL_OBJ) $(PARSER_OBJ) $(AST_OBJ) $(COMPILER_OBJ)
	g++ $(CFLAGS) $(LDFLAGS) -o$@ $^

parser/lexer.cc parser/parser.cc: parser/lexer.l parser/parser.y
	(cd parser && flex lexer.l && bison -d --color=always -ggraph -oparser.cc parser.y)

%.o: %.cc
	g++ $(CFLAGS) -o$@ -c $<

clean:
	rm -rf *.o
	rm -rf ast/*.o
	rm -rf util/*.o
	rm -rf parser/lexer.cc parser/parser.cc
	rm -rf parser/*.hh parser/graph
	rm -rf parser/*.o
	rm -rf $(TARGET)
