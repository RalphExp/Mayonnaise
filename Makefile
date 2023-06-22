TARGET=may

CFLAGS = -g -I. -Iinclude -std=c++11

LDFLAGS =

PARSER_OBJ = \
    $(patsubst %.cc, %.o, $(wildcard parser/*.cc)) \
    $(patsubst %.l, %.o, $(wildcard parser/*.l)) \
    $(patsubst %.y, %.o, $(wildcard parser/*.y))

AST_OBJ = $(patsubst %.cc, %.o, $(wildcard ast/*.cc))
IR_OBJ = $(patsubst %.cc, %.o, $(wildcard ir/*.cc))
COMPILER_OBJ = $(patsubst %.cc, %.o, $(wildcard compiler/*.cc))
UTIL_OBJ = $(patsubst %.cc, %.o, $(wildcard util/*.cc))
ENTITY_OBJ = $(patsubst %.cc, %.o, $(wildcard entity/*.cc))
MAIN_OBJ = $(patsubst %.cc, %.o, $(wildcard *.cc))

$(TARGET): $(UTIL_OBJ) $(AST_OBJ) $(ENTITY_OBJ) $(PARSER_OBJ) $(COMPILER_OBJ) $(IR_OBJ) $(MAIN_OBJ)
	g++ $(CFLAGS) -o$@ $^

parser/lexer.cc parser/parser.cc: parser/lexer.l parser/parser.y
	@(cd parser && flex lexer.l && bison -d --color=always -oparser.cc parser.y)

%.o: %.cc
	g++ $(CFLAGS) -o$@ -c $<

clean:
	rm -rf *.o
	rm -rf ast/*.o
	rm -rf util/*.o
	rm -rf entity/*.o
	rm -rf parser/lexer.cc parser/parser.cc
	rm -rf parser/*.hh parser/graph
	rm -rf parser/*.o
	rm -rf $(TARGET)
