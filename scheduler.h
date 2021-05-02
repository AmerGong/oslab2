#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <queue>
#include <stack>
#include <string>

#include "event.h"
#include "process.h"

class scheduler {
   public:
    scheduler(int quantum, int max_prio, const std::string& name);
    virtual ~scheduler();
    virtual void add_pcb(PCB* pcb) = 0;
    virtual PCB* get_next_pcb() = 0;
    virtual bool test_preempt(PCB* pcb, int curtime);
    int get_quantum() const;
    const std::string& get_name() const;

   protected:
    int quantum;
    int max_prio;
    std::string name;
};

class fcfs : public scheduler {
   public:
    fcfs();
    virtual ~fcfs() override;
    virtual void add_pcb(PCB* pcb) override;
    virtual PCB* get_next_pcb() override;

   private:
    std::queue<PCB*> ready_que;
};

class lcfs : public scheduler {
   public:
    lcfs();
    virtual ~lcfs() override;
    virtual void add_pcb(PCB* pcb) override;
    virtual PCB* get_next_pcb() override;

   private:
    std::stack<PCB*> ready_que;
};

// struct pPCB_cmp {
//     bool operator()(const PCB* pcb1, const PCB* pcb2) const;
// };

class srtf : public scheduler {
   public:
    srtf();
    virtual ~srtf() override;
    virtual void add_pcb(PCB* pcb) override;
    virtual PCB* get_next_pcb() override;

   private:
    class pcb_pack {
       public:
        pcb_pack(PCB* pcb);
        bool operator<(const pcb_pack& rhs) const;

        PCB* pcb;

       private:
        int index;
        static int pcb_pack_index;
    };

    std::priority_queue<pcb_pack> ready_que;
};

class rr : public scheduler {
   public:
    rr(int quantum);
    virtual ~rr() override;
    virtual void add_pcb(PCB* pcb) override;
    virtual PCB* get_next_pcb() override;

   private:
    std::queue<PCB*> ready_que;
};

class prio : public scheduler {
   public:
    prio(int quantum, int max_prio);
    virtual ~prio() override;
    virtual void add_pcb(PCB* pcb) override;
    virtual PCB* get_next_pcb() override;

   private:
    int active_size;
    int expired_size;
    std::queue<PCB*>* active_que;
    std::queue<PCB*>* expired_que;
};

class preprio : public scheduler {
   public:
    preprio(int quantum, int max_prio);
    virtual ~preprio() override;
    virtual void add_pcb(PCB* pcb) override;
    virtual PCB* get_next_pcb() override;
    virtual bool test_preempt(PCB* pcb, int curtime) override;

   private:
    int active_size;
    int expired_size;
    std::queue<PCB*>* active_que;
    std::queue<PCB*>* expired_que;
};

#endif  // __SCHEDULER_H__
