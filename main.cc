#include <unistd.h>
#include <getopt.h>
#include <string>
#include <vector>

#include "may.h"

using namespace std;

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

    while (true) {
        c = getopt_long(argc, argv, "h", long_options, &opt_index);
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

    FILE* f = fopen(argv[optind], "r");
    yyin = f;
    yyparse();
    return 0;
}
