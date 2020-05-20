.PHONY=clean run clearlogs

default: libintercept.so

run: libintercept.so
	./permutate.sh -c './run.sh' -f '-w #site'

libintercept.so: chrome_intercept.cc experiment/cpu_utils.* experiment/experimenter.*
	clang++ -g -shared -fPIC -ldl -lg3logger -o libintercept.so chrome_intercept.cc experiment/cpu_utils.cc experiment/experimenter.cc

clean:
	-@rm *.so *.out 2>/dev/null || true

clearlogs:
	-@rm logs/*.log 2>/dev/null || true
