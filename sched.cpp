#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cassert>
#include <getopt.h>

#include "debug.h"
#include "simulator.h"

using namespace std;

#ifdef ENABLE_RUNTIME_DEBUG_LEVEL
#ifdef DEBUG
int debug_level = 4;
#else
int debug_level = 1;
#endif
#endif

int parse_args(int argc, char **argv, string &spec, bool &v, bool &t, bool &e) {
    const char *optstr = "s:vte";

    int ch;

    v = t = e = false;

    opterr = 0; /* if set to 1, error msg will be printed to stderr */
    while ((ch = getopt(argc, argv, optstr)) != -1) {
        switch (ch) {
            case 's':
                spec = optarg;
                break;
            case 'v':
                v = true;
                break;
            case 't':
                t = true;
                break;
            case 'e':
                e = true;
                break;
            default:
                if (optopt == 's') {
                    fprintf(stderr, "missing argument option: %c\n",
                            (char)optopt);
                } else {
                    fprintf(stderr, "unknown option: %c\n", (char)optopt);
                }
                break;
        }
    }

    stmt_debug(print_debug("Remain args:\n");
               for (int i = optind; i < argc;
                    ++i) { print_debug_ex("\t|%s|\n", argv[i]); });

    return optind;
}

int main(int argc, char **argv) {
    stmt_debug(print_debug("ARGS:\n"); for (int i = 1; i < argc; ++i) {
        print_debug_ex("\t|%s|\n", argv[i]);
    });

    string spec;
    bool v, t, e;
    int ind = parse_args(argc, argv, spec, v, t, e);
    if (argc - ind != 2) {
        cerr << "Missing arguments!" << endl;
        cerr << "Usage: " << argv[0]
             << " [-v] [-t] [-e][-s<schedspec>] inputfile "
                "randfile"
             << endl;
        exit(1);
    }
    simulator sim(spec, argv[ind], argv[ind + 1], v, t, e);

    // sim.test_event_queue();

    sim.start();

    return 0;
}
