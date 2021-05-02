#include <fstream>
#include <iostream>
#include <cassert>

#include "random.h"
#include "debug.h"

using namespace std;

my_random::my_random(const std::string &rfile) : ofs(0) {
    print_debug("Random: %s\n", rfile.c_str());
    ifstream input(rfile);
    if (!input) {
        cerr << "Can't open file: " << rfile << endl;
        exit(1);
    }
    input >> n_randvals;
    print_debug_ex("\t%d\n", n_randvals);
    randvals = new int[n_randvals];
    assert(randvals);
    for (int i = 0; i < n_randvals; ++i) {
        input >> randvals[i];
    }
    assert(!input.bad());
}

int my_random::myrandom(int burst) {
    if (ofs == n_randvals) ofs = 0;
    return 1 + (randvals[ofs++] % burst);
}

my_random::~my_random() { delete[] randvals; }
