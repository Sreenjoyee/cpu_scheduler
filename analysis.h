// Analysis features for the menu (each function is marked with its owner)
#pragma once
#include "advanced.h"

// MEMBER 3 — rank all algorithms on a chosen criterion and recommend the best one
void best_algorithm(const vector<P>& ps, int q, int crit) {
    const char* names[] = {"", "Average Waiting Time", "Average Turnaround Time", "Average Response Time",
                           "Fairness", "Context Switches", "Balanced (all criteria)"};
    vector<pair<string, Summary>> res;
    for (auto& [name, f] : ALGOS(q)) res.push_back({name, metrics(ps, f(ps))});
    auto value = [&](const Summary& s, int c) {              // smaller = better for every criterion
        double v[] = {0, s.wt, s.tat, s.rt, -s.fair, s.ctx};
        return v[c];
    };
    map<string, double> score;                               // balanced = sum of ranks on criteria 1-5
    for (int c = 1; c <= 5; c++) {
        auto r = res;
        sort(r.begin(), r.end(), [&](auto& a, auto& b) { return value(a.second, c) < value(b.second, c); });
        for (size_t i = 0; i < r.size(); i++) score[r[i].first] += i + 1;
    }
    auto key = [&](auto& x) { return crit == 6 ? score[x.first] : value(x.second, crit); };
    stable_sort(res.begin(), res.end(), [&](auto& a, auto& b) { return key(a) < key(b); });
    printf("\nRanking by %s:\n", names[crit]);
    for (size_t i = 0; i < res.size(); i++)
        printf("%2zu. %-16s %8.3f\n", i + 1, res[i].first.c_str(), crit == 4 ? -key(res[i]) : key(res[i]));
    cout << "\nRecommended: " << res[0].first << "\n";
}

// MEMBER 2 — effect of the time quantum on Round Robin
void quantum_analysis(const vector<P>& ps) {
    int maxbt = 0;
    for (auto& p : ps) maxbt = max(maxbt, p.bt);
    printf("\n%8s%8s%8s%8s%7s\n", "Quantum", "AvgWT", "AvgTAT", "AvgRT", "CtxSw");
    int best = 1;
    double bestTat = 1e18;
    for (int q = 1; q <= maxbt; q++) {
        Summary s = metrics(ps, round_robin(ps, q));
        printf("%8d%8.2f%8.2f%8.2f%7.0f\n", q, s.wt, s.tat, s.rt, s.ctx);
        if (s.tat < bestTat) bestTat = s.tat, best = q;
    }
    cout << "\nBest quantum for turnaround time: " << best
         << "\nSmall quantum -> fast response but many context switches."
            "\nQuantum >= largest burst -> Round Robin becomes FCFS.\n";
}

// MEMBER 1 — every context switch costs `cs` time units (same order of execution, CPU idle while switching)
Gantt with_overhead(Gantt g, int cs) {
    int t = 0;
    string prev;
    for (auto& x : g) {
        int len = x.e - x.s;
        x.s = max(x.s, t + (prev.empty() || prev == x.pid ? 0 : cs));
        x.e = x.s + len;
        t = x.e, prev = x.pid;
    }
    return g;
}

void overhead_analysis(const vector<P>& ps, int q, int cs) {
    printf("\nContext switch cost = %d time unit(s)\n%-16s%8s%8s%7s%10s%10s\n",
           cs, "Algorithm", "AvgWT", "AvgTAT", "CtxSw", "CPU% (0)", "CPU% (cs)");
    for (auto& [name, f] : ALGOS(q)) {
        Gantt g = f(ps);
        Summary a = metrics(ps, g), b = metrics(ps, with_overhead(g, cs));
        printf("%-16s%8.2f%8.2f%7.0f%10.1f%10.1f\n", name.c_str(), b.wt, b.tat, b.ctx, a.util, b.util);
    }
    cout << "Algorithms with many context switches (RR, MLFQ) lose the most CPU time.\n";
}
