// MEMBER 3 — Multilevel Queue, Multilevel Feedback Queue, Multiprocessor, Thread scheduling
#pragma once
#include "basic.h"
#include "priority_rr.h"

// 3 levels: level 0 RR q=2, level 1 RR q=4, level 2 FCFS. A higher level always preempts a lower one.
// MLQ : process stays in its fixed queue p.q.   MLFQ: starts at level 0, moves down after a full quantum.
Gantt multilevel(vector<P> ps, bool feedback) {
    const int Q[3] = {2, 4, INT_MAX};
    stable_sort(ps.begin(), ps.end(), [](auto& a, auto& b) { return a.at < b.at; });
    int n = ps.size(), t = 0, i = 0, cur = -1, used = 0, back = -1;
    vector<int> lvl(n);
    deque<int> qs[3];
    auto top = [&] { for (int l = 0; l < 3; l++) if (!qs[l].empty()) return l; return 3; };
    Gantt g;
    while (i < n || cur >= 0 || back >= 0 || top() < 3) {
        for (; i < n && ps[i].at <= t; i++) qs[lvl[i] = feedback ? 0 : min(ps[i].q, 2)].push_back(i);
        if (back >= 0) qs[lvl[back]].push_back(back), back = -1;           // quantum expired
        if (cur >= 0 && top() < lvl[cur]) qs[lvl[cur]].push_front(cur), cur = -1;   // preempted
        if (cur < 0) {
            if (top() == 3) { t = ps[i].at; continue; }
            cur = qs[top()].front(), qs[top()].pop_front(), used = 0;
        }
        add(g, ps[cur].pid, t, t + 1);
        t++, used++, ps[cur].bt--;
        if (!ps[cur].bt) cur = -1;
        else if (used == Q[lvl[cur]]) {
            if (feedback) lvl[cur] = min(lvl[cur] + 1, 2);
            back = cur, cur = -1;
        }
    }
    return g;
}
Gantt mlq(const vector<P>& ps)  { return multilevel(ps, false); }
Gantt mlfq(const vector<P>& ps) { return multilevel(ps, true); }

// Symmetric multiprocessing, one common ready queue (FCFS): the CPU that is free first takes the next job.
vector<Gantt> multiprocessor(const vector<P>& ps, int ncpu) {
    vector<int> freeAt(ncpu, 0);
    vector<Gantt> g(ncpu);
    for (auto& p : fcfs(ps)) {                          // FCFS order of jobs
        int c = min_element(freeAt.begin(), freeAt.end()) - freeAt.begin();
        int at = 0, bt = 0;
        for (auto& x : ps) if (x.pid == p.pid) at = x.at, bt = x.bt;
        int s = max(freeAt[c], at);
        g[c].push_back({p.pid, s, s + bt});
        freeAt[c] = s + bt;
    }
    return g;
}

// Thread ids look like "A.T1" (process A). SCS (one-to-one): kernel round-robins all threads.
// PCS (many-to-one): kernel round-robins processes; a process's slice is shared by its own threads.
Gantt thread_scheduling(vector<P> th, int q, bool scs) {
    if (scs) return round_robin(th, q);
    vector<P> procs;                                    // one entry per process: total burst
    map<string, deque<int>> mine;
    for (int k = 0; k < (int)th.size(); k++) {
        string pr = th[k].pid.substr(0, th[k].pid.find('.'));
        if (mine[pr].empty()) procs.push_back({pr, th[k].at, 0});
        for (auto& p : procs) if (p.pid == pr) p.bt += th[k].bt, p.at = min(p.at, th[k].at);
        mine[pr].push_back(k);
    }
    Gantt g;
    for (auto& x : round_robin(procs, q))               // kernel's schedule of processes
        for (int t = x.s; t < x.e; t++) {               // thread library: rotate threads every unit
            auto& d = mine[x.pid];
            int k = d.front(); d.pop_front();
            add(g, th[k].pid, t, t + 1);
            if (--th[k].bt) d.push_back(k);
        }
    return g;
}

// all single-CPU algorithms (used by main and tests)
vector<pair<string, Algo>> ALGOS(int q = 2) {
    return {{"FCFS", fcfs}, {"SJF", sjf}, {"SRTF", srtf}, {"PRIORITY-NP", priority_np},
            {"PRIORITY-P", priority_p}, {"PRIORITY-AGING", priority_aging},
            {"RR", [q](const vector<P>& ps) { return round_robin(ps, q); }}, {"MLQ", mlq}, {"MLFQ", mlfq}};
}
