#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <string>
#include <vector>

struct process {
    int at;  // arrival time
    int tc;  // cpu time
    int cb;  // cpu burst
    int io;  // io burst
    process(int at, int tc, int cb, int io);
};

typedef enum {
    STATE_CREATED,
    STATE_READY,
    STATE_RUNNING,
    STATE_BLOCKED,
    STATE_DONE
} process_state_t;

std::ostream &operator<<(std::ostream &os, process_state_t state);

class event;
class PCB {
   public:
    PCB(process *proc, int static_prio);
    process *get_proc() const;
    process_state_t get_state() const;
    void set_state(process_state_t state, int timestamp);
    event *get_event() const;
    int get_state_timestamp() const;
    int get_pid() const;
    void set_event(event *evt);

   private:
    process *proc;
    process_state_t state;
    int state_ts;  // state timestamp
    int pid;
    event *evt;

    static int pid_index;

   public:
    int static_prio;
    int dynamic_prio;

    int cpu_burst;
    int io_burst;

    int cpu_time;  // time in ready state
    int io_time;   // time in blocked state
    
    int remain_time;
};

std::vector<process> read_processes(const std::string &path);

#endif  // __PROCESS_H__
