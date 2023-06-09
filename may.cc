#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <set>

#include <fcntl.h>

#include "ast.h"
#include "util.h"
#include "option.h"
#include "loader.h"

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
    bool dump_expr = false;
    bool dump_stmt = false;
    bool dump_token = false;

    while ((c = getopt_long(argc, argv, "hat", long_options, &opt_index)) != -1) {
        switch (c) {
        case -1:
            break;
        case 'h':
            usage(argv[0]);
            break;
        case 't':
            dump_token = true;
            break;
        case 'a':
            dump_ast = true;
            break;
        }
    }

    if (optind == argc) {
        usage(argv[0]);
    }

    for (; argv[optind] != nullptr; ++optind) {
        int fd = open(argv[optind], O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "can not open file %s\n", argv[optind]);
            exit(1);
        }
        fprintf(stdout, "processing file %s\n", argv[optind]);

        Option option;
        yyscan_t lexer;
        yylex_init(&lexer);
        yyset_extra(&option, lexer);
        option.src_ = argv[optind];
        option.start_ = parser::Parser::token::COMPILE;

        FILE* f = fdopen(fd, "r");
        yyset_in(f, lexer);
 
        if (dump_token == true) {
            cbc_dump_token(lexer);
            fclose(f);
            yylex_destroy(lexer);
            continue;
        }

        try {
            parser::Parser parser(lexer);
            int res = parser.parse();
            if (res == 0) {   
                cbc::AST* ast = option.ast_;
                // AST will be destroyed when option is out of the current scope
                if (dump_ast) {
                    Dumper dumper(cout);
                    ast->dump(dumper);
                }
                ast->dec_ref();
            }
        } catch (...) {
            // printf("error: %s\n", e.c_str());
        }

        fclose(f);
        close(fd);
        yylex_destroy(lexer);
    }
    return 0;
}

