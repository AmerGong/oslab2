#include <fstream>
#include <iostream>
#include <cassert>

#include "process.h"
#include "debug.h"

using namespace std;

int PCB::pid_index = 0;

process::process(int at, int tc, int cb, int io)
    : at(at), tc(tc), cb(cb), io(io) {}

ostream &operator<<(ostream &os, process_state_t state) {
    static string state_strs[] = {"CREATED", "READY", "RUNNG", "BLOCK", "Done"};
    os << state_strs[(int)state];
    return os;
}

PCB::PCB(process *proc, int static_prio)
    : proc(proc),
      state(STATE_CREATED),
      state_ts(proc->at),
      pid(pid_index++),
      evt(NULL),
      static_prio(static_prio),
      dynamic_prio(static_prio - 1),
      cpu_burst(0),
      io_burst(0),
      cpu_time(0),
      io_time(0),
      remain_time(proc->tc) {}

process *PCB::get_proc() const { return proc; }

process_state_t PCB::get_state() const { return state; }

void PCB::set_state(process_state_t state, int timestamp) {
    this->state = state;
    this->state_ts = timestamp;
}

event *PCB::get_event() const { return evt; }

int PCB::get_state_timestamp() const { return state_ts; }

int PCB::get_pid() const { return pid; }

void PCB::set_event(event *evt) { this->evt = evt; }

std::vector<process> read_processes(const std::string &path) {
    print_debug("read_processes: %s\n", path.c_str());
    ifstream input(path);
    if (!input) {
        cerr << "Can't open file: " << path << endl;
        exit(1);
    }

    int at;  // arrival time
    int tc;  // cpu time
    int cb;  // cpu burst
    int io;  // io burst
    std::vector<process> vec;
    while (input >> at >> tc >> cb >> io) {
        vec.emplace_back(at, tc, cb, io);
    }
    assert(!input.bad());
    return vec;
}
