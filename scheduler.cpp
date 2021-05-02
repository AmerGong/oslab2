#include <fstream>
#include <iostream>
#include <cassert>

#include "scheduler.h"

using namespace std;

int srtf::pcb_pack::pcb_pack_index = 0;

scheduler::scheduler(int quantum, int max_prio, const std::string& name)
    : quantum(quantum), max_prio(max_prio), name(name) {
    assert(quantum > 0);
}

scheduler::~scheduler() {}

bool scheduler::test_preempt(PCB* pcb, int curtime) {
    (void)pcb;
    (void)curtime;
    return false;
}

int scheduler::get_quantum() const { return quantum; }

const std::string& scheduler::get_name() const { return name; }

/*==========================================================================*/

fcfs::fcfs() : scheduler(INT32_MAX, 4, "FCFS") {}

fcfs::~fcfs() {}

void fcfs::add_pcb(PCB* pcb) { ready_que.push(pcb); }

PCB* fcfs::get_next_pcb() {
    PCB* pcb = NULL;
    if (ready_que.size()) {
        pcb = ready_que.front();
        ready_que.pop();
    }
    return pcb;
}

/*==========================================================================*/

lcfs::lcfs() : scheduler(INT32_MAX, 4, "LCFS") {}

lcfs::~lcfs() {}

void lcfs::add_pcb(PCB* pcb) { ready_que.push(pcb); }

PCB* lcfs::get_next_pcb() {
    PCB* pcb = NULL;
    if (ready_que.size()) {
        pcb = ready_que.top();
        ready_que.pop();
    }
    return pcb;
}

/*==========================================================================*/

// bool pPCB_cmp::operator()(const PCB* pcb1, const PCB* pcb2) const {
//     return pcb1->remain_time < pcb2->remain_time;
// }

srtf::pcb_pack::pcb_pack(PCB* pcb) : pcb(pcb), index(pcb_pack_index++) {}

bool srtf::pcb_pack::operator<(const pcb_pack& rhs) const {
    return pcb->remain_time != rhs.pcb->remain_time
               ? pcb->remain_time > rhs.pcb->remain_time
               : index > rhs.index;
}

srtf::srtf() : scheduler(INT32_MAX, 4, "SRTF") {}

srtf::~srtf() {}

void srtf::add_pcb(PCB* pcb) { ready_que.push(pcb); }

PCB* srtf::get_next_pcb() {
    PCB* pcb = NULL;
    if (ready_que.size()) {
        pcb = ready_que.top().pcb;
        ready_que.pop();
    }
    return pcb;
}

/*==========================================================================*/

rr::rr(int quantum) : scheduler(quantum, 4, "RR") {}

rr::~rr() {}

void rr::add_pcb(PCB* pcb) { ready_que.push(pcb); }

PCB* rr::get_next_pcb() {
    PCB* pcb = NULL;
    if (ready_que.size()) {
        pcb = ready_que.front();
        ready_que.pop();
    }
    return pcb;
}

/*==========================================================================*/

prio::prio(int quantum, int max_prio)
    : scheduler(quantum, max_prio, "PRIO"), active_size(0), expired_size(0) {
    active_que = new std::queue<PCB*>[max_prio];
    expired_que = new std::queue<PCB*>[max_prio];
    assert(active_que);
    assert(expired_que);
}

prio::~prio() {
    delete[] active_que;
    delete[] expired_que;
}

void prio::add_pcb(PCB* pcb) {
    if (pcb->dynamic_prio == -1) {
        pcb->dynamic_prio = pcb->static_prio - 1;
        expired_que[pcb->dynamic_prio].push(pcb);
        expired_size++;
    } else {
        active_que[pcb->dynamic_prio].push(pcb);
        active_size++;
    }
}

PCB* prio::get_next_pcb() {
    if (active_size == 0) {
        if (expired_size == 0) return NULL;
        std::swap(active_que, expired_que);
        std::swap(expired_size, active_size);
    }

    PCB* pcb = NULL;
    for (int i = max_prio - 1; i >= 0; --i) {
        if (active_que[i].size()) {
            pcb = active_que[i].front();
            active_que[i].pop();
            break;
        }
    }
    active_size--;
    return pcb;
}

/*==========================================================================*/

preprio::preprio(int quantum, int max_prio)
    : scheduler(quantum, max_prio, "PREPRIO"), active_size(0), expired_size(0) {
    active_que = new std::queue<PCB*>[max_prio];
    expired_que = new std::queue<PCB*>[max_prio];
    assert(active_que);
    assert(expired_que);
}

preprio::~preprio() {
    delete[] active_que;
    delete[] expired_que;
}

void preprio::add_pcb(PCB* pcb) {
    if (pcb->dynamic_prio == -1) {
        pcb->dynamic_prio = pcb->static_prio - 1;
        expired_que[pcb->dynamic_prio].push(pcb);
        expired_size++;
    } else {
        active_que[pcb->dynamic_prio].push(pcb);
        active_size++;
    }
}

PCB* preprio::get_next_pcb() {
    if (active_size == 0) {
        if (expired_size == 0) return NULL;
        std::swap(active_que, expired_que);
        std::swap(expired_size, active_size);
    }

    PCB* pcb = NULL;
    for (int i = max_prio - 1; i >= 0; --i) {
        if (active_que[i].size()) {
            pcb = active_que[i].front();
            active_que[i].pop();
            break;
        }
    }
    active_size--;
    return pcb;
}

bool preprio::test_preempt(PCB* pcb, int curtime) {
    return pcb && pcb->get_event()->get_time() != curtime;
}
