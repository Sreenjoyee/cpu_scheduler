// MEMBER 4 — efficiency: average results over 100 test cases + running time (menu option 3)
#pragma once
#include "cases.h"

void efficiency_report() {
    printf("Average over 100 test cases\n%-16s%8s%8s%8s%7s%7s   Time(ms) n=1000\n",
           "Algorithm", "AvgWT", "AvgTAT", "AvgRT", "Fair", "CtxSw");
    vector<P> big = random_case(1000, 7);
    for (auto& [name, f] : ALGOS()) {
        Summary avg;
        for (int seed = 1; seed <= 100; seed++) {
            vector<P> ps = test_case(seed);
            Summary s = metrics(ps, f(ps));
            avg.wt += s.wt / 100, avg.tat += s.tat / 100, avg.rt += s.rt / 100;
            avg.fair += s.fair / 100, avg.ctx += s.ctx / 100;
        }
        auto t0 = chrono::steady_clock::now();
        f(big);
        double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();
        printf("%-16s%8.2f%8.2f%8.2f%7.2f%7.2f%12.2f\n", name.c_str(), avg.wt, avg.tat, avg.rt, avg.fair, avg.ctx, ms);
    }
}
