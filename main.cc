#include <unistd.h>
#include <getopt.h>
#include <stdio.h>

void yyset_in(FILE* file);

extern "C" {
	int yyparse(void);
}

static struct option long_options[] = {
    // options: name, has_args, flag, val
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

void usage()
{
    
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
            usage();
            break;
        }
    }

    if (optind == argc) {
        usage();
    }

    printf("argv[%d] = %s\n", optind, argv[optind]);
    FILE* f = fopen(argv[optind], "r");
    // yyset_in(f);
    yyparse();
    return 0;
}
