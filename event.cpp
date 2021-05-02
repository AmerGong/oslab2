#include <fstream>
#include <iostream>
#include <cassert>

#include "event.h"
#include "debug.h"

using namespace std;

int event::event_index = 0;

event::event(int time, event_trans_t trans, PCB* pcb)
    : index(event_index++), time(time), trans(trans), pcb(pcb) {}

bool event::operator<(const event& rhs) const {
    return time != rhs.time ? time > rhs.time : index > rhs.index;
}

int event::get_time() const { return time; }

event_trans_t event::get_transition() const { return trans; }

PCB* event::get_pcb() const { return pcb; }

void event::disable() { pcb = NULL; }

bool event::is_disabled() const { return pcb == NULL; }
