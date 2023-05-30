#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <set>

#include "parser/lexer.hh"
#include "parser/parser.hh"

using namespace std;

static struct option long_options[] = {
    // options: name, has_args, flag, val (return from getopt_long)
    {"help", no_argument, 0, 'h'},
    {"dump-ast", no_argument, 0, 'a'},
    {"dump-tokens", no_argument, 0, 't'},
    {0, 0, 0, 0}
};

void usage(const char* name)
{
    printf("usage: %s [options] file...\n", name);
    printf("global options:\n");
    printf("  --dump-tokens    dump tokens and quit.\n");
    printf("  --dump-ast       dump ast and quit.\n");
    exit(1);
}

void cbc_dump_token(yyscan_t lexer)
{
    int c = 0;
    do {
        parser::Parser::location_type loc;
        parser::Parser::semantic_type val;
        c = yylex(&val, &loc, lexer);
        auto tok = val.as<Token>();
        printf("token: %-15s at line: %d column: %d\n",
            tok.image_.c_str(), tok.begin_line_, tok.begin_column_);
    } while (c != 0);
}

int main(int argc, char *argv[])
{
    int c;
    int opt_index = 0;
    bool dump_ast = false;
    bool dump_token = false;

    while ((c = getopt_long(argc, argv, "ht", long_options, &opt_index)) != -1) {
        switch (c) {
        case -1:
            break;
        case 'h':
            usage(argv[0]);
            break;
        case 'a':
            dump_ast = true;
        case 't':
            dump_token = true;
            break;
        }
    }

    if (optind == argc) {
        usage(argv[0]);
    }

    printf("argv[%d] = %s\n", optind, argv[optind]);

    FILE* f = fopen(argv[optind], "r");
    yyscan_t lexer;
    set<string> typenames;
    yylex_init(&lexer);
    yyset_in(f, lexer);
    yyset_extra(&typenames, lexer);

    if (dump_token == true) {
        cbc_dump_token(lexer);
        yylex_destroy(lexer);
        exit(0);
    }

    try {
        parser::Parser parser(lexer);
        parser.parse();
    } catch (...) {
        // printf("error: %s\n", e.c_str());
    }
    return 0;
}