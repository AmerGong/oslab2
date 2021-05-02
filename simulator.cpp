#include <fstream>
#include <iostream>
#include <iomanip>
#include <cassert>

#include "simulator.h"
#include "event.h"
#include "debug.h"

using namespace std;

bool pevent_cmp::operator()(const event *evt1, const event *evt2) {
    return *evt1 < *evt2;
}

simulator::simulator(const std::string &spec, const std::string &pfile,
                     const std::string &rfile, bool v, bool t, bool e)
    : myrand(rfile), procs(read_processes(pfile)), v(v), t(t), e(e) {
    int quantum, max_prio = 4;
    switch (spec[0]) {
        case 'F':
            sched = new fcfs();
            break;
        case 'L':
            sched = new lcfs();
            break;
        case 'S':
            sched = new srtf();
            break;
        case 'R':
            assert(sscanf(spec.c_str() + 1, "%d", &quantum) == 1);
            sched = new rr(quantum);
            break;
        case 'P':
            assert(sscanf(spec.c_str() + 1, "%d:%d", &quantum, &max_prio) >= 1);
            sched = new prio(quantum, max_prio);
            break;
        case 'E':
            assert(sscanf(spec.c_str() + 1, "%d:%d", &quantum, &max_prio) >= 1);
            sched = new preprio(quantum, max_prio);
            break;
        default:
            cerr << "Wrong schedspec: " << spec << endl;
            exit(1);
    }
    for (auto &p : procs) {
        auto pcb = new PCB(&p, myrand.myrandom(max_prio));
        pcbs.push_back(pcb);
        event *evt = new event(p.at, TRANS_TO_READY, pcb);
        event_que.push(evt);
    }
}

simulator::~simulator() {
    for (auto &pcb : pcbs) {
        delete pcb;
    }
    delete sched;
}

void simulator::start() {
    event *evt, *new_evt;
    PCB *running_pcb = NULL;
    bool CALL_SCHEDULER = false;
    int last = -1;
    int io_time = 0, io_cnt = 0, io_farest = -1;

    while (event_que.size()) {
        evt = event_que.top();
        event_que.pop();

        if (evt->is_disabled()) continue;

        // this is the process pcb the event works on
        PCB *pcb = evt->get_pcb();

        int runtime, time;
        process *proc = pcb->get_proc();
        int CURRENT_TIME = evt->get_time();
        int time_in_prev_state = CURRENT_TIME - pcb->get_state_timestamp();
        auto old_state = pcb->get_state();

        switch (evt->get_transition()) {  // which state to transition to?
            case TRANS_TO_READY:
                // must come from BLOCKED or from PREEMPTION
                // must add to run queue
                if (pcb->get_state() == STATE_BLOCKED) {
                    pcb->io_time += time_in_prev_state;
                    pcb->dynamic_prio = pcb->static_prio - 1;
                    io_cnt--;
                }

                if (sched->test_preempt(running_pcb, CURRENT_TIME) &&
                    running_pcb->dynamic_prio < pcb->dynamic_prio) {
                    running_pcb->get_event()->disable();
                    new_evt =
                        new event(CURRENT_TIME, TRANS_TO_PREEMPT, running_pcb);
                    running_pcb->set_event(new_evt);
                    event_que.push(new_evt);
                }

                pcb->set_state(STATE_READY, CURRENT_TIME);
                sched->add_pcb(pcb);

                // conditional on whether something is run
                CALL_SCHEDULER = true;
                break;
            case TRANS_TO_RUN:
                // must come from READY
                pcb->cpu_time += time_in_prev_state;

                // create event for either preemption or blocking
                if (pcb->cpu_burst == 0) {
                    pcb->cpu_burst =
                        min(myrand.myrandom(proc->cb), pcb->remain_time);
                }
                pcb->set_state(STATE_RUNNING, CURRENT_TIME);
                runtime = min(pcb->cpu_burst, sched->get_quantum());
                time = CURRENT_TIME + runtime;
                running_pcb = pcb;

                if (pcb->remain_time <= runtime) {
                    new_evt = new event(time, TRANS_TO_DONE, pcb);
                } else if (sched->get_quantum() < pcb->cpu_burst) {
                    new_evt = new event(time, TRANS_TO_PREEMPT, pcb);
                } else {
                    new_evt = new event(time, TRANS_TO_BLOCK, pcb);
                }
                pcb->set_event(new_evt);
                event_que.push(new_evt);
                break;
            case TRANS_TO_BLOCK:
                // must come from RUNNING
                pcb->remain_time -= time_in_prev_state;
                pcb->cpu_burst -= time_in_prev_state;

                // create an event for when process becomes READY again
                pcb->io_burst = myrand.myrandom(proc->io);
                pcb->set_state(STATE_BLOCKED, CURRENT_TIME);
                time = CURRENT_TIME + pcb->io_burst;
                if (io_cnt) {
                    if (io_farest < time) {
                        io_time += time - io_farest;
                        io_farest = time;
                    }
                } else {
                    io_time += pcb->io_burst;
                    io_farest = time;
                }
                io_cnt++;
                running_pcb = NULL;

                new_evt = new event(time, TRANS_TO_READY, pcb);
                pcb->set_event(new_evt);
                event_que.push(new_evt);

                CALL_SCHEDULER = true;
                break;
            case TRANS_TO_PREEMPT:
                // add to runqueue (no event is generated)
                pcb->remain_time -= time_in_prev_state;
                pcb->cpu_burst -= time_in_prev_state;
                running_pcb = NULL;

                pcb->dynamic_prio--;

                pcb->set_state(STATE_READY, CURRENT_TIME);
                sched->add_pcb(pcb);
                CALL_SCHEDULER = true;
                break;
            case TRANS_TO_DONE:
                pcb->set_state(STATE_DONE, time);
                running_pcb = NULL;
                last = time;

                CALL_SCHEDULER = true;
                break;
        }

        if (v) {
            if (pcb->get_state() == STATE_DONE) {
                cout << pcb->get_state_timestamp() << " " << pcb->get_pid()
                     << " " << pcb->cpu_burst << ": " << pcb->get_state()
                     << endl;
            } else {
                cout << CURRENT_TIME << " " << pcb->get_pid() << " "
                     << time_in_prev_state << ": " << old_state << " -> "
                     << pcb->get_state();
                if (old_state == STATE_RUNNING &&
                    pcb->get_state() == STATE_READY) {
                    cout << "  cb=" << pcb->cpu_burst
                         << " rem=" << pcb->remain_time
                         << " prio=" << pcb->dynamic_prio;
                } else if (pcb->get_state() == STATE_RUNNING) {
                    cout << " cb=" << pcb->cpu_burst
                         << " rem=" << pcb->remain_time
                         << " prio=" << pcb->dynamic_prio;
                } else if (pcb->get_state() == STATE_BLOCKED) {
                    cout << "  ib=" << pcb->io_burst
                         << " rem=" << pcb->remain_time;
                }
                cout << endl;
            }
        }

        // remove current event object from Memory
        delete evt;

        if (CALL_SCHEDULER) {
            if (get_next_event_time() == CURRENT_TIME) {
                continue;  // process next event from Event queue
            }
            CALL_SCHEDULER = false;  // reset global flag
            if (running_pcb == NULL) {
                running_pcb = sched->get_next_pcb();
                if (running_pcb) {
                    // create event to make this process runnable for same time.
                    new_evt =
                        new event(CURRENT_TIME, TRANS_TO_RUN, running_pcb);
                    running_pcb->set_event(new_evt);
                    event_que.push(new_evt);
                }
            }
        }
    }
    print_result(last, io_time);
}

void simulator::test_event_queue() {
    while (event_que.size()) {
        event *evt = event_que.top();
        cout << "Time: " << evt->get_time()
             << " cb:" << evt->get_pcb()->get_proc()->cb << endl;
        event_que.pop();
    }
}

int simulator::get_next_event_time() const {
    return event_que.size() ? event_que.top()->get_time() : -1;
}

void simulator::print_result(int last, int io_time) {
    cout << sched->get_name();
    if (sched->get_quantum() != INT32_MAX) cout << " " << sched->get_quantum();
    cout << endl;
    // pid: AT TC CB IO PRIO | FT TT IT CW
    int cpu_time = 0;  // cpu running time
    // int io_time = 0;   // io blocked time
    int turn_around_time = 0;
    int cpu_waiting_time = 0;
    for (auto &pcb : pcbs) {
        assert(pcb->get_state() == STATE_DONE);
        // os << setw(4) << setfill('0') << pcb->get_pid() << setfill(' ') << ":
        // "
        //    << setw(4) << pcb->get_proc()->at << " " << setw(4)
        //    << pcb->get_proc()->tc << " " << setw(4) << pcb->get_proc()->cb
        //    << " " << setw(4) << pcb->get_proc()->io << " " <<
        //    pcb->static_prio;
        // os << " | " << setw(5) << pcb->get_state_timestamp() << " " <<
        // setw(5)
        //    << pcb->get_state_timestamp() - pcb->get_proc()->at << " " <<
        //    setw(5)
        //    << pcb->io_time << " " << setw(5) << pcb->cpu_time << endl;
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", pcb->get_pid(),
               pcb->get_proc()->at, pcb->get_proc()->tc, pcb->get_proc()->cb,
               pcb->get_proc()->io, pcb->static_prio,
               pcb->get_state_timestamp(),
               pcb->get_state_timestamp() - pcb->get_proc()->at, pcb->io_time,
               pcb->cpu_time);

        cpu_time += pcb->get_state_timestamp() - pcb->get_proc()->at -
                    pcb->cpu_time - pcb->io_time;
        // io_time += pcb->io_time;
        turn_around_time += pcb->get_state_timestamp() - pcb->get_proc()->at;
        cpu_waiting_time += pcb->cpu_time;
    }

    // os << fixed << setprecision(2);
    // os << "SUM: " << last << " " << (double)cpu_time * 100 / last << " "
    //    << (double)io_time * 100 / last << " "
    //    << (double)turn_around_time / pcbs.size() << " "
    //    << (double)cpu_waiting_time / pcbs.size() << " " << setprecision(3)
    //    << (double)pcbs.size() * 100 / last << endl;
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", last,
           (double)cpu_time * 100 / last, (double)io_time * 100 / last,
           (double)turn_around_time / pcbs.size(),
           (double)cpu_waiting_time / pcbs.size(),
           (double)pcbs.size() * 100 / last);
}
