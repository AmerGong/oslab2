#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include <string>
#include "random.h"
#include "event.h"
#include "process.h"
#include "scheduler.h"

struct pevent_cmp {
    bool operator()(const event *evt1, const event *evt2);
};

class simulator {
   public:
    simulator(const std::string &spec, const std::string &pfile,
              const std::string &rfile, bool v, bool t, bool e);
    ~simulator();
    void start();
    void test_event_queue();

   private:
    int get_next_event_time() const;
    void print_result(int last, int io_time);

   private:
    my_random myrand;
    std::vector<process> procs;
    bool v, t, e;

    scheduler *sched;
    std::vector<PCB *> pcbs;
    std::priority_queue<event *, std::vector<event *>, pevent_cmp> event_que;
};

#endif  // __SIMULATOR_H__
