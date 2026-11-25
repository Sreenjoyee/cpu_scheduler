// MEMBER 1 — Process, scheduling engines, metrics, Gantt chart
#pragma once
#include <bits/stdc++.h>
using namespace std;

struct P { string pid; int at, bt, pr = 0, q = 0; };   // arrival, burst, priority (low = high), queue
struct Seg { string pid; int s, e; };                   // one Gantt chart block
using Gantt = vector<Seg>;
using Algo = function<Gantt(const vector<P>&)>;

void validate(const vector<P>& ps) {
    set<string> ids;
    for (auto& p : ps)
        if (!ids.insert(p.pid).second || p.at < 0 || p.bt <= 0 || p.pr < 0 || p.q < 0)
            throw invalid_argument("invalid process " + p.pid);
}

vector<P> load(const string& file) {                   // line: pid arrival burst [priority] [queue]
    ifstream f(file);
    if (!f) throw runtime_error("cannot open " + file);
    vector<P> ps;
    string line;
    while (getline(f, line)) {
        stringstream ss(line);
        P p;
        if (line[0] != '#' && ss >> p.pid >> p.at >> p.bt) { ss >> p.pr >> p.q; ps.push_back(p); }
    }
    validate(ps);
    return ps;
}

void add(Gantt& g, const string& pid, int s, int e) {   // merge with previous block if same process
    if (!g.empty() && g.back().pid == pid && g.back().e == s) g.back().e = e;
    else g.push_back({pid, s, e});
}

// Generic engine. key(p, remaining, t) picks the process (ties: earlier arrival, then input order).
// preemptive = re-decide every time unit, otherwise run the chosen process to completion.
Gantt schedule(const vector<P>& ps, function<int(const P&, int, int)> key, bool preemptive) {
    int n = ps.size(), t = 0, left = n;
    vector<int> rem(n);
    for (int i = 0; i < n; i++) rem[i] = ps[i].bt;
    Gantt g;
    while (left) {
        int b = -1, next = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (!rem[i]) continue;
            next = min(next, ps[i].at);
            if (ps[i].at <= t && (b < 0 || make_tuple(key(ps[i], rem[i], t), ps[i].at, i) <
                                           make_tuple(key(ps[b], rem[b], t), ps[b].at, b))) b = i;
        }
        if (b < 0) { t = next; continue; }               // CPU idle -> jump to next arrival
        int run = preemptive ? 1 : rem[b];
        add(g, ps[b].pid, t, t + run);
        t += run, rem[b] -= run;
        if (!rem[b]) left--;
    }
    return g;
}

struct Row { string pid; int at, bt, ct, tat, wt, rt; };
struct Summary { double wt = 0, tat = 0, rt = 0, maxwt = 0, util = 0, thru = 0, fair = 0, ctx = 0; };

Summary metrics(const vector<P>& ps, const Gantt& g, vector<Row>* rows = nullptr, int ncpu = 1) {
    Summary s;
    if (ps.empty()) return s;
    double busy = 0, sx = 0, sxx = 0;
    int lo = INT_MAX, hi = 0, n = ps.size();
    for (auto& x : g) busy += x.e - x.s;
    for (auto& p : ps) {
        int st = INT_MAX, ct = 0;
        for (auto& x : g) if (x.pid == p.pid) st = min(st, x.s), ct = max(ct, x.e);
        Row r{p.pid, p.at, p.bt, ct, ct - p.at, ct - p.at - p.bt, st - p.at};
        if (rows) rows->push_back(r);
        s.wt += r.wt, s.tat += r.tat, s.rt += r.rt, s.maxwt = max(s.maxwt, (double)r.wt);
        lo = min(lo, p.at), hi = max(hi, ct);
        double x = (double)p.bt / r.tat;                 // 1 = never waited
        sx += x, sxx += x * x;
    }
    s.wt /= n, s.tat /= n, s.rt /= n;
    s.util = 100 * busy / ((hi - lo) * ncpu);
    s.thru = (double)n / (hi - lo);
    s.fair = sx * sx / (n * sxx);                       // Jain's fairness index (1 = fair)
    s.ctx = max((int)g.size() - ncpu, 0);
    return s;
}

void print(const vector<P>& ps, Gantt g, int ncpu = 1) {
    string top = "|", bot = "0";
    auto cell = [&](const string& name, int end) {
        string e = to_string(end);
        int w = max(name.size(), e.size()) + 2, l = (w - name.size()) / 2;
        top += string(l, ' ') + name + string(w - l - name.size(), ' ') + "|";
        bot += string(w + 1 - e.size(), ' ') + e;
    };
    int t = 0;
    sort(g.begin(), g.end(), [](auto& a, auto& b) { return a.s < b.s; });
    for (auto& x : g) {
        if (x.s > t) cell("idle", x.s);
        cell(x.pid, x.e);
        t = x.e;
    }
    if (ncpu == 1) cout << top << "\n" << bot << "\n\n";
    vector<Row> rows;
    Summary s = metrics(ps, g, &rows, ncpu);
    printf("PID      AT   BT   CT  TAT   WT   RT\n");
    for (auto& r : rows) printf("%-6s%5d%5d%5d%5d%5d%5d\n", r.pid.c_str(), r.at, r.bt, r.ct, r.tat, r.wt, r.rt);
    printf("Avg WT=%.2f  Avg TAT=%.2f  Avg RT=%.2f  CPU=%.1f%%  Throughput=%.3f  Fairness=%.3f  CtxSwitches=%.0f\n",
           s.wt, s.tat, s.rt, s.util, s.thru, s.fair, s.ctx);
}
