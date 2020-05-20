.PHONY=clean run clearlogs

default: libintercept.so

run: libintercept.so
	./permutate.sh -c './run.sh'

libintercept.so: chrome_intercept.cc experiment/NanoLog/libnanolog.so experiment/cpu_utils.* experiment/experimenter.*
	clang++ -g -shared -fPIC -ldl -lnanolog -Lexperiment/NanoLog/ -o libintercept.so chrome_intercept.cc experiment/cpu_utils.cc experiment/experimenter.cc

experiment/NanoLog/libnanolog.so:
	g++ -fPIC -pthread -shared experiment/NanoLog/NanoLog.cpp -o experiment/NanoLog/libnanolog.so

clean:
	-@rm *.so *.out 2>/dev/null || true
	-@rm experiment/NanoLog/*.so *.so 2>/dev/null || true

clearlogs:
	-@rm logs/*.txt 2>/dev/null || true
