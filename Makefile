.PHONY=clean run

default: libintercept.so

run: libintercept.so
	./run.sh

libintercept.so: chrome_intercept.cc experiment/NanoLog/libnanolog.so experiment/cpu_utils.* experiment/experimenter.*
	clang++ -g -shared -fPIC -ldl -lnanolog -Lexperiment/NanoLog/ -o libintercept.so chrome_intercept.cc experiment/cpu_utils.cc experiment/experimenter.cc

experiment/NanoLog/libnanolog.so:
	g++ -fPIC -pthread -shared experiment/NanoLog/NanoLog.cpp -o experiment/NanoLog/libnanolog.so

clean:
	-@rm *.so *.out 2>/dev/null || true
	-@rm experiment/NanoLog/*.so *.so 2>/dev/null || true
