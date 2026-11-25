// MEMBER 4 — edge cases, 100 test cases, starvation (menu option 11, or ./tests_run)
#pragma once
#include "cases.h"

int passed = 0, failed = 0;
void check(bool ok, const string& what) { if (ok) passed++; else failed++, cout << "FAIL: " << what << "\n"; }
bool same(const Gantt& a, const Gantt& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) if (a[i].pid != b[i].pid || a[i].s != b[i].s || a[i].e != b[i].e) return false;
    return true;
}
double wt(const vector<P>& ps, const Gantt& g) { return round(metrics(ps, g).wt * 100) / 100; }

bool valid(const vector<P>& ps, Gantt g) {                // rules every schedule must follow
    map<string, int> need, got;
    for (auto& p : ps) need[p.pid] = p.bt;
    for (auto& x : g) {
        for (auto& p : ps) if (p.pid == x.pid && x.s < p.at) return false;   // ran before arrival
        got[x.pid] += x.e - x.s;
    }
    sort(g.begin(), g.end(), [](auto& a, auto& b) { return a.s < b.s; });
    for (size_t i = 1; i < g.size(); i++) if (g[i - 1].e > g[i].s) return false;   // overlap
    return got == need;                                    // exactly its burst time
}

// LONG (burst 10, low priority, batch queue) + N short jobs (burst 2, high priority, system queue)
// arriving every 2 units -> there is always a short job ready. Prints the table, returns verdicts.
map<string, bool> starvation_report() {
    cout << "Starvation test: waiting time of LONG job\n";
    printf("%-16s%8s%8s%8s   Verdict\n", "Algorithm", "N=10", "N=100", "N=200");
    map<string, bool> verdict;
    for (auto& [name, f] : ALGOS()) {
        int w[3], N[3] = {10, 100, 200};
        for (int k = 0; k < 3; k++) {
            vector<P> ps = {{"LONG", 0, 10, 5, 2}};
            for (int i = 0; i < N[k]; i++) ps.push_back({"S" + to_string(i + 1), 2 * i, 2, 1, 0});
            vector<Row> rows;
            metrics(ps, f(ps), &rows);
            w[k] = rows[0].wt;
        }
        bool s = verdict[name] = w[2] >= 0.9 * 2 * 200;    // waited for (almost) all short jobs
        printf("%-16s%8d%8d%8d   %s\n", name.c_str(), w[0], w[1], w[2], s ? "STARVATION" : "ok");
    }
    cout << "Starving algorithms: the LONG job's wait keeps growing as more short jobs arrive.\n";
    return verdict;
}

int run_tests() {
    passed = failed = 0;
    // ---------- 1. Edge cases ----------
    for (auto& [name, f] : ALGOS()) {
        check(f({}).empty(), "empty input " + name);
        check(same(f({{"P1", 3, 5}}), {{"P1", 3, 8}}), "single process " + name);
        check(same(f({{"P1", 5, 2}, {"P2", 20, 3}}), {{"P1", 5, 7}, {"P2", 20, 23}}), "idle CPU " + name);
    }
    vector<P> tie = {{"P1", 0, 4}, {"P2", 0, 4}, {"P3", 0, 4}};
    check(same(sjf(tie), fcfs(tie)), "tie broken by input order");
    check(same(srtf({{"P1", 0, 4}, {"P2", 4, 2}}), {{"P1", 0, 4}, {"P2", 4, 6}}), "arrives exactly at finish");
    vector<P> a = {{"P1", 0, 5, 2}, {"P2", 1, 3, 2}, {"P3", 2, 8, 2}};
    check(same(priority_np(a), fcfs(a)), "equal priorities = FCFS");
    check(same(round_robin(a, 100), fcfs(a)), "huge quantum = FCFS");
    check(round_robin({{"P1", 0, 3}, {"P2", 0, 3}}, 1).size() == 6, "quantum 1");
    check(sjf({{"P1", 0, 1000000}, {"P2", 1, 1}}).back().e == 1000001, "very large burst");
    check(same(mlq({{"P1", 0, 3, 0, 9}}), {{"P1", 0, 3}}), "queue number out of range");
    vector<P> th = {{"A.T1", 0, 4}, {"A.T2", 0, 4}, {"A.T3", 0, 4}, {"B.T1", 0, 4}};
    check(metrics(th, thread_scheduling(th, 1, false)).maxwt < 15, "PCS: B gets half the CPU");
    for (vector<P> bad : vector<vector<P>>{{{"P1", -1, 3}}, {{"P1", 0, 0}}, {{"P1", 0, 3}, {"P1", 1, 2}}}) {
        bool thrown = false;
        try { validate(bad); } catch (...) { thrown = true; }
        check(thrown, "invalid input rejected");
    }

    // ---------- 2. The 100 test cases (tests/cases/) x every algorithm ----------
    for (int seed = 1; seed <= 100; seed++) {
        vector<P> ps = test_case(seed);
        double best = wt(ps, srtf(ps));
        for (auto& [name, f] : ALGOS()) {
            Gantt g = f(ps);
            string id = "case " + to_string(seed) + " " + name;
            check(valid(ps, g), id + " invalid schedule");
            check(g.back().e == fcfs(ps).back().e, id + " CPU idle while work was ready");
            check(wt(ps, g) >= best, id + " beat SRTF (impossible, SRTF is optimal)");
        }
        for (auto& g : multiprocessor(ps, 3)) {
            vector<P> mine;
            for (auto& p : ps) for (auto& x : g) if (x.pid == p.pid) mine.push_back(p);
            check(valid(mine, g), "case " + to_string(seed) + " MULTI");
        }
    }

    // ---------- 3. Starvation ----------
    set<string> starves = {"SJF", "SRTF", "PRIORITY-NP", "PRIORITY-P", "MLQ", "MLFQ"};
    for (auto& [name, s] : starvation_report()) check(s == (bool)starves.count(name), name + " starvation");
    cout << "\n" << passed << " checks passed, " << failed << " failed\n";
    return failed > 0;
}
