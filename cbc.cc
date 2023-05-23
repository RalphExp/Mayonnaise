#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "scanner.hh"
#include "parser.hh"

using namespace std;

static struct option long_options[] = {
    // options: name, has_args, flag, val
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

void cbc_dump_token(yyscan_t scanner)
{
    int c = 0;
    do {
        yy::Parser::location_type loc;
        yy::Parser::semantic_type val;
        c = yylex(&val, &loc, scanner);
        if (c < 256)
            printf("token: %-15c at line: %4d column: %3d\n",
                val.as<Token>().kind_, loc.begin.line, loc.begin.column);
        else
            printf("token: %-15s at line: %4d column: %3d\n",
                val.as<Token>().image_.c_str(), loc.begin.line, loc.begin.column);
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
    yyscan_t scanner;
    yylex_init(&scanner);
    yyset_in(f, scanner);

    if (dump_token == true) {
        cbc_dump_token(scanner);
        yylex_destroy(scanner);
        exit(0);
    }

    yy::Parser parser(scanner);
    parser.parse();
    return 0;
}
