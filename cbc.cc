#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "scanner.h"
#include "parser.hh"

using namespace std;

static struct option long_options[] = {
    // options: name, has_args, flag, val
    {"help", no_argument, 0, 'h'},
    {"dump-tokens", no_argument, 0, 0},
    {0, 0, 0, 0}
};

void usage(const char* name)
{
    printf("usage: %s [options] file...\n", name);
    printf("global options:\n");
    printf("  --dump-tokens    dump tokens and quit.\n");
    exit(1);
}

void cbc_dump_token(yyscan_t scanner)
{
    int c = 0;
    do {
        yy::Parser::location_type loc;
        yy::Parser::semantic_type val;
        c = yylex(&val, &loc, scanner);
        switch (c) {
        case yy::Parser::token::STRING:
            printf("token: \"%s\"\n", val.as<string>().c_str());
            break;
        case yy::Parser::token::CHARACTER:
            printf("token: '%s'\n", val.as<string>().c_str());
            break;
        case yy::Parser::token::IDENTIFIER:
        case yy::Parser::token::INTEGER:
            printf("token: %s\n", val.as<string>().c_str());
            break;
        default:
            if (c > 255)
                printf("token: %d\n", c);
            else
                printf("token: %c\n", c);
        }
    } while (c != 0);
}

int main(int argc, char *argv[])
{
    int c;
    int opt_index = 0;
    bool dump_token = false;

    while ((c = getopt_long(argc, argv, "h", long_options, &opt_index)) != -1) {
        switch (c) {
        case 0:
            if (opt_index == 1) {
                dump_token = true;
            }
            break;
        case -1:
            break;
        case 'h':
            usage(argv[0]);
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
