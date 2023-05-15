#include <unistd.h>
#include <getopt.h>
#include <string>
#include <vector>

using namespace std;

static struct option long_options[] = {
    // options: name, has_args, flag, val
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};


int main(int argc, char *argv[])
{
    int c;
    int opt_index;

    vector<string> files;

    while (true) {
        c = getopt_long(argc, argv, "h", long_options, &opt_index);
        if (c == -1)
            break;
    }

    while (optind < argc) {
        files.push_back(argv[optind]);
        ++optind;
    }
    return 0;
}
