// MEMBER 2 — menu-driven program.   Run: ./scheduler   (or ./scheduler file.txt to start with a file)
#include "analysis.h"
#include "tests/efficiency.h"
#include "tests/tests.h"

int readInt(const string& msg) {                       // keeps asking until a number is typed
    int x;
    while (cout << msg, !(cin >> x)) {
        if (cin.eof()) exit(0);                        // Ctrl+D
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Please enter a number.\n";
    }
    return x;
}

vector<P> input_processes() {
    vector<P> ps;
    int n = readInt("Number of processes: ");
    cout << "For each process enter: arrival burst priority queue(0/1/2)\n";
    for (int i = 0; i < n; i++) {
        P p{"P" + to_string(i + 1), 0, 0};
        while (true) {
            p.at = readInt(p.pid + ": "), p.bt = readInt(""), p.pr = readInt(""), p.q = readInt("");
            if (p.at >= 0 && p.bt > 0 && p.pr >= 0 && p.q >= 0) break;
            cout << "Arrival, priority, queue must be >= 0 and burst > 0. Enter again.\n";
        }
        ps.push_back(p);
    }
    return ps;
}

void show(const vector<P>& ps) {
    printf("\nPID      AT   BT   PR    Q\n");
    for (auto& p : ps) printf("%-6s%5d%5d%5d%5d\n", p.pid.c_str(), p.at, p.bt, p.pr, p.q);
}

void compare(const vector<P>& ps, int q) {
    show(ps);
    printf("\n    %-16s%8s%8s%8s%7s%7s%7s\n", "Algorithm", "AvgWT", "AvgTAT", "AvgRT", "MaxWT", "Fair", "CtxSw");
    auto algos = ALGOS(q);
    for (size_t i = 0; i < algos.size(); i++) {
        Summary s = metrics(ps, algos[i].second(ps));
        printf("%2zu. %-16s%8.2f%8.2f%8.2f%7.0f%7.3f%7.0f\n", i + 1, algos[i].first.c_str(),
               s.wt, s.tat, s.rt, s.maxwt, s.fair, s.ctx);
    }
    cout << "10. MULTIPROCESSOR\n11. THREAD SCHEDULING (SCS vs PCS)\n";
    while (int c = readInt("\nSee Gantt chart of algorithm (1-11, 0 = back): ")) {
        if (c >= 1 && c <= 9) {
            cout << "\n=== " << algos[c - 1].first << " ===\n";
            print(ps, algos[c - 1].second(ps));
        } else if (c == 10) {
            int cpus = readInt("Number of CPUs: ");
            if (cpus < 1) continue;
            vector<Gantt> g = multiprocessor(ps, cpus);
            Gantt all;
            for (int k = 0; k < cpus; k++) {
                cout << "CPU " << k << ": ";
                for (auto& x : g[k]) cout << x.pid << "[" << x.s << "-" << x.e << "] ";
                cout << "\n";
                all.insert(all.end(), g[k].begin(), g[k].end());
            }
            print(ps, all, cpus);
        } else if (c == 11) {
            cout << "(Thread ids like A.T1, A.T2 = threads of process A. Try: ./scheduler samples/threads.txt)\n";
            cout << "\n=== SCS ===\n"; print(ps, thread_scheduling(ps, q, true));
            cout << "\n=== PCS ===\n"; print(ps, thread_scheduling(ps, q, false));
        }
    }
}

int main(int argc, char* argv[]) {
    vector<P> ps;
    int q = 2;
    if (argc > 1) {
        try { ps = load(argv[1]); cout << "Loaded " << ps.size() << " processes from " << argv[1] << "\n"; }
        catch (exception& e) { cout << "Error: " << e.what() << "\n"; }
    }
    while (true) {
        cout << "\n========= CPU Scheduling Simulator =========\n"
                "--- Processes ---\n"
                " 1. Enter processes manually\n"
                " 2. Load processes from file / test case\n"
                " 3. Generate random processes\n"
                " 4. View / save current processes\n"
                "--- Scheduling ---\n"
                " 5. Algorithm comparison and results\n"
                " 6. Recommend best algorithm\n"
                " 7. Round Robin time quantum analysis\n"
                " 8. Context switch overhead analysis\n"
                "--- Testing ---\n"
                " 9. Starvation demonstration\n"
                "10. Efficiency report (100 test cases)\n"
                "11. Run all tests (edge cases + 100 test cases + starvation)\n"
                " 0. Exit\n"
                "Current: " << ps.size() << " processes, RR quantum = " << q << "\n";
        int ch = readInt("Choice: ");
        bool needPs = ch >= 4 && ch <= 8;
        if (needPs && ps.empty()) { cout << "No processes yet. Use option 1, 2 or 3 first.\n"; continue; }
        try {
            if (ch == 0) break;
            else if (ch == 1 || ch == 3) {
                if (ch == 1) ps = input_processes();
                else {
                    int n = readInt("Number of processes: "), at = readInt("Max arrival time: "), bt = readInt("Max burst time: ");
                    if (n < 1 || at < 0 || bt < 1) { cout << "Invalid values.\n"; continue; }
                    ps = random_case(n, time(0), at, bt);
                    show(ps);
                }
                q = readInt("Time quantum for Round Robin: ");
                if (q < 1) q = 2, cout << "Invalid quantum, using 2.\n";
            } else if (ch == 2) {
                cout << "Enter a file name (e.g. samples/basic.txt) or a test case number 1-100: ";
                string s;
                cin >> s;
                ps = all_of(s.begin(), s.end(), ::isdigit) ? test_case(stoi(s)) : load(s);
                show(ps);
            } else if (ch == 4) {
                show(ps);
                cout << "Save to file? Enter file name (or - to skip): ";
                string s;
                cin >> s;
                if (s != "-") {
                    ofstream f(s);
                    f << "# pid arrival burst priority queue\n";
                    for (auto& p : ps) f << p.pid << " " << p.at << " " << p.bt << " " << p.pr << " " << p.q << "\n";
                    cout << (f ? "Saved to " + s : "Could not write " + s) << "\n";
                }
            } else if (ch == 5) compare(ps, q);
            else if (ch == 6) {
                cout << "Criterion: 1.Waiting  2.Turnaround  3.Response  4.Fairness  5.Context switches  6.Balanced\n";
                int c = readInt("Choice: ");
                if (c >= 1 && c <= 6) best_algorithm(ps, q, c); else cout << "Invalid criterion.\n";
            } else if (ch == 7) quantum_analysis(ps);
            else if (ch == 8) {
                int cs = readInt("Time for one context switch: ");
                if (cs >= 0) overhead_analysis(ps, q, cs); else cout << "Must be >= 0.\n";
            } else if (ch == 9) { cout << "\n"; starvation_report(); }
            else if (ch == 10) { cout << "\n"; efficiency_report(); }
            else if (ch == 11) { cout << "\n"; run_tests(); }
            else cout << "Invalid choice.\n";
        } catch (exception& e) { cout << "Error: " << e.what() << " (run from the project folder)\n"; }
    }
}
