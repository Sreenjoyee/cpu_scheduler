# CPU Scheduling Simulator and Performance Analyzer
Operating System (24B11CS312) — JIIT Noida, CSE, Batch F12 — C++17, Ubuntu

## Project structure
```
cpu_scheduler/
├── common.h          Process struct, scheduling engine, metrics, Gantt chart       (Member 1)
├── basic.h           FCFS, SJF, SRTF                                               (Member 1)
├── priority_rr.h     Priority NP / P / Aging, Round Robin                          (Member 2)
├── main.cpp          Menu, process input / load / save                             (Member 2)
├── advanced.h        MLQ, MLFQ, Multiprocessor, Thread scheduling                  (Member 3)
├── analysis.h        overhead_analysis (M1), quantum_analysis (M2), best_algorithm (M3)
├── tests/
│   ├── cases.h       Test case loader + random process generator                  (Member 4)
│   ├── tests.h       Edge cases, 100 test cases, starvation                       (Member 4)
│   ├── tests.cpp     Standalone test runner (./tests_run)                         (Member 4)
│   ├── efficiency.h  Efficiency report                                            (Member 4)
│   └── cases/        case_001.txt ... case_100.txt                                (Member 4)
├── samples/          basic.txt, threads.txt                                       (Member 4)
└── Makefile                                                                       (Member 4)
```

## Work division

### 1. Sreenjoyee Saha (992401030087) — Core engine + basic algorithms
- **Files:** `common.h`, `basic.h`, `with_overhead()` / `overhead_analysis()` in `analysis.h`
- **Built:** process structure, scheduling engine, metric formulas (CT, TAT, WT, RT, CPU %, throughput, fairness), Gantt chart, FCFS, SJF, SRTF, context switch overhead
- **Presents:** option 5 (comparison table + Gantt charts 1–3: FCFS, SJF, SRTF) and option 8 (context switch overhead)

### 2. Sanskriti Negi (992401030102) — Priority + Round Robin + menu
- **Files:** `priority_rr.h`, `main.cpp`, `quantum_analysis()` in `analysis.h`
- **Built:** Priority (non-preemptive, preemptive), Priority with aging (starvation fix), Round Robin, menu, process input / load / save, quantum analysis
- **Presents:** options 1, 2, 4 (enter, load, view/save processes), option 5 Gantt charts 4–7 (Priority NP, Priority P, Priority Aging, RR) and option 7 (RR quantum analysis)

### 3. Niyati Kaushik (992401030098) — Advanced scheduling
- **Files:** `advanced.h`, `best_algorithm()` in `analysis.h`
- **Built:** Multilevel Queue, Multilevel Feedback Queue, Multiprocessor scheduling, Thread scheduling (PCS vs SCS), best-algorithm recommendation
- **Presents:** option 5 Gantt charts 8–11 (MLQ, MLFQ, multiprocessor, threads — start with `./scheduler samples/threads.txt` for 11) and option 6 (recommend best algorithm)

### 4. Mahima Show (992401030099) — Testing + efficiency
- **Files:** everything in `tests/`, `samples/`, `Makefile`
- **Built:** random process generator, edge-case tests, 100 test cases, starvation test, efficiency report
- **Presents:** option 3 (random processes), option 9 (starvation), option 10 (efficiency report), option 11 (run all tests)

### Demo order
| Step | Presenter | Menu options |
|---|---|---|
| 1 | Sanskriti | Start the program, explain the menu, options 1, 2, 4 |
| 2 | Sreenjoyee | Option 5 (table + FCFS, SJF, SRTF), option 8 |
| 3 | Sanskriti | Option 5 (Priority NP, P, Aging, RR), option 7 |
| 4 | Niyati | Option 5 (MLQ, MLFQ, multiprocessor, threads), option 6 |
| 5 | Mahima | Options 3, 9, 10, 11 |

## How to run (Ubuntu)
```bash
sudo apt install g++ make            # only if not installed
make                                 # builds scheduler and tests_run

./scheduler                          # menu-driven program
./scheduler samples/basic.txt        # same menu, processes already loaded from a file
./scheduler samples/threads.txt      # use with option 5 -> 11 for thread scheduling
./tests_run                          # same as menu option 11, without the menu
```

Menu:
```
--- Processes ---
 1. Enter processes manually            -> arrival, burst, priority, queue for each + RR quantum
 2. Load processes from file / test case-> e.g. samples/basic.txt, or 42 for tests/cases/case_042.txt
 3. Generate random processes           -> choose count, max arrival, max burst
 4. View / save current processes       -> print the table, optionally save it to a file
--- Scheduling ---
 5. Algorithm comparison and results    -> table of all algorithms, then pick 1-11 for its Gantt chart
 6. Recommend best algorithm            -> ranks all algorithms by WT / TAT / RT / fairness / switches / balanced
 7. Round Robin time quantum analysis   -> runs RR for quantum 1..max burst, shows the trade-off
 8. Context switch overhead analysis    -> each switch costs N time units; shows WT, TAT and CPU% lost
--- Testing ---
 9. Starvation demonstration
10. Efficiency report (100 test cases)
11. Run all tests (edge cases + 100 test cases + starvation)
 0. Exit
```
Run it from the project folder so options 2, 10 and 11 can find `tests/cases/`.

Input file: one process per line, `pid arrival burst [priority] [queue]`. Lower number = higher priority.
MLQ/MLFQ queues: 0 = RR q=2, 1 = RR q=4, 2 = FCFS.

## Testing
**Edge cases:** empty input, a single process, idle CPU, ties, arrival exactly at another's finish, all priorities equal, RR with a huge quantum and with quantum 1, a burst of 10^6, a queue number out of range, PCS threads, invalid input (negative arrival, zero burst, duplicate PID).

**100 test cases:** the files `tests/cases/case_001.txt` … `case_100.txt` (1–15 processes each; load any one with `./scheduler tests/cases/case_042.txt`) run through all 9 algorithms and the multiprocessor scheduler. Each schedule is checked to make sure that (3,047 checks in total, all passing):
- each process gets exactly its burst time
- no process runs before it arrives
- no two processes are on one CPU at the same time
- the CPU is never idle while work is waiting
- no algorithm beats SRTF on average waiting time (SRTF is optimal)

**Starvation:** test case = one LONG job (burst 10, low priority, batch queue) plus N short jobs (burst 2, high priority) arriving every 2 units.

| Algorithm | WT of LONG (N=10 / 100 / 200) | Result |
|---|---|---|
| FCFS | 0 / 0 / 0 | ok |
| SJF, SRTF | 20 / 200 / 400 | **starves** (always a shorter job) |
| Priority NP, P | 20 / 200 / 400 | **starves** (always a higher-priority job) |
| Priority + Aging | 20 / 25 / 25 | fixed by aging |
| Round Robin | 20 / 28 / 28 | ok |
| MLQ | 20 / 200 / 400 | **starves** (batch queue never runs) |
| MLFQ | 20 / 200 / 400 | **starves** (LONG pushed to the bottom queue) |

**Efficiency (average over the 100 test cases):**

| Algorithm | Avg WT | Avg TAT | Avg RT | Fairness | Ctx switches | Time for 1000 processes (ms) |
|---|---|---|---|---|---|---|
| FCFS | 13.06 | 18.49 | 13.06 | 0.71 | 6.90 | 2.1 |
| SJF | 9.42 | 14.85 | 9.42 | 0.80 | 6.90 | 2.2 |
| SRTF | **8.96** | **14.39** | 8.30 | 0.83 | 7.79 | 8.6 |
| Priority NP | 12.90 | 18.32 | 12.90 | 0.72 | 6.90 | 2.3 |
| Priority P | 13.30 | 18.73 | 11.57 | 0.72 | 7.95 | 11.2 |
| Priority Aging | 14.26 | 19.69 | 10.36 | 0.73 | 15.28 | 16.0 |
| RR | 17.67 | 23.09 | 4.31 | **0.86** | 21.85 | 0.1 |
| MLQ | 14.43 | 19.85 | 10.30 | 0.75 | 13.32 | 0.1 |
| MLFQ | 17.79 | 23.21 | **1.56** | 0.82 | 16.34 | 0.2 |

SRTF gives the lowest waiting time. RR and MLFQ respond fastest but switch the most. FCFS switches least but is the least fair. (Timings depend on the machine, so run menu option 10 and use your own numbers.)