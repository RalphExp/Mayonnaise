#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" {
    #include "may.h"
    int yylex(void);
    void yyset_in(FILE*);
}

extern FILE *yyin;
extern YYSTYPE yylval;

static struct option long_options[] = {
    // options: name, has_args, flag, val
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

void usage(const char* name)
{
    printf("usage: %s [files]\n", name);
    exit(1);
}

int main(int argc, char *argv[])
{
    int c;
    int opt_index;

    while (c = getopt_long(argc, argv, "h", long_options, &opt_index) != -1) {
        switch (c) {
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
    yyset_in(f);
    yyparse();

    /* to distinct with ASCII, TOKEN start from 256*/
    /*
    while (true) {
        
        int c = yylex();
        // printf("%d, %d\n", c, yylval.i_value);
    }
    */
    return 0;
}
