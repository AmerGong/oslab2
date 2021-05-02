#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <string>

class my_random {
   public:
    my_random(const std::string &rfile);
    int myrandom(int burst);
    ~my_random();

   private:
    int ofs;
    int n_randvals;
    int *randvals;
};

#endif  // __RANDOM_H__
