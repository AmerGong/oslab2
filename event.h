#ifndef __EVENT_H__
#define __EVENT_H__

#include "process.h"

enum event_trans_t {
    TRANS_TO_READY,
    TRANS_TO_RUN,
    TRANS_TO_BLOCK,
    TRANS_TO_PREEMPT,
    TRANS_TO_DONE
};

class event {
   public:
    event(int time, event_trans_t trans, PCB *pcb);
    bool operator<(const event &rhs) const;
    int get_time() const;
    event_trans_t get_transition() const;
    PCB *get_pcb() const;
    void disable();
    bool is_disabled() const;

   private:
    int index;  // make sure stable sort
    int time;
    event_trans_t trans;
    PCB *pcb;

    static int event_index;
};

#endif  // __EVENT_H__
