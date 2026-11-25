// MEMBER 2 — Priority (non-preemptive, preemptive, with aging) and Round Robin
#pragma once
#include "common.h"

Gantt priority_np(const vector<P>& ps) { return schedule(ps, [](const P& p, int, int) { return p.pr; }, false); }
Gantt priority_p(const vector<P>& ps)  { return schedule(ps, [](const P& p, int, int) { return p.pr; }, true); }

// Starvation fix: priority improves by 1 for every 5 units spent waiting
Gantt priority_aging(const vector<P>& ps) {
    return schedule(ps, [](const P& p, int rem, int t) { return p.pr - (t - p.at - (p.bt - rem)) / 5; }, true);
}

Gantt round_robin(vector<P> ps, int q = 2) {
    stable_sort(ps.begin(), ps.end(), [](auto& a, auto& b) { return a.at < b.at; });
    int n = ps.size(), t = 0, i = 0;
    deque<int> rq;
    Gantt g;
    auto arrive = [&] { while (i < n && ps[i].at <= t) rq.push_back(i++); };
    while (i < n || !rq.empty()) {
        arrive();
        if (rq.empty()) { t = ps[i].at; continue; }
        int k = rq.front(); rq.pop_front();
        int run = min(q, ps[k].bt);
        add(g, ps[k].pid, t, t + run);
        t += run, ps[k].bt -= run;          // bt used as remaining time (ps is a copy)
        arrive();                           // new arrivals join before the preempted process
        if (ps[k].bt) rq.push_back(k);
    }
    return g;
}
