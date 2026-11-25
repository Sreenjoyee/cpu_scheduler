// MEMBER 4 — test case loading and random process generation
#pragma once
#include "../advanced.h"

vector<P> test_case(int k) {                             // tests/cases/case_001.txt ... case_100.txt
    char name[64];
    snprintf(name, sizeof name, "tests/cases/case_%03d.txt", k);
    return load(name);
}

vector<P> random_case(int n, unsigned seed, int maxAt = 20, int maxBt = 10) {
    mt19937 r(seed);
    vector<P> ps(n);
    for (int i = 0; i < n; i++)
        ps[i] = {"P" + to_string(i + 1), int(r() % (maxAt + 1)), int(1 + r() % maxBt), int(r() % 6), int(r() % 3)};
    return ps;
}
