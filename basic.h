// MEMBER 1 — FCFS, SJF (non-preemptive), SRTF
#pragma once
#include "common.h"

Gantt fcfs(const vector<P>& ps) { return schedule(ps, [](const P& p, int, int) { return p.at; }, false); }
Gantt sjf(const vector<P>& ps)  { return schedule(ps, [](const P& p, int, int) { return p.bt; }, false); }
Gantt srtf(const vector<P>& ps) { return schedule(ps, [](const P&, int rem, int) { return rem; }, true); }
