all:
	g++ -std=c++17 -O2 -Wall main.cpp -o scheduler
	g++ -std=c++17 -O2 -Wall tests/tests.cpp -o tests_run

clean:
	rm -f scheduler tests_run
