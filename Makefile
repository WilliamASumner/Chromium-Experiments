.PHONY=clean run-sh run-verbose run-gui clear-logs run-pychrome run-debug

SITE_PREFIX:=+site
DEFINES=

default: bin/libintercept-new.so
	mv bin/libintercept-new.so bin/libintercept.so

run-sh: logs/ bin/libintercept-old.so
	cp bin/libintercept-old.so bin/libintercept.so
	./src/scripts/permutate.sh -c './src/scripts/run-chrome.sh' -f '-w +url -v' -p '$(SITE_PREFIX)'

run-gui: logs/ bin/libintercept-new.so
	cp bin/libintercept-new.so bin/libintercept.so
	./chrome-experimenter.py --gui

run-pychrome: logs/ bin/libintercept-new.so
	cp bin/libintercept-new.so bin/libintercept.so
	./chrome-experimenter.py --verbose --plot-graphs

run-debug: logs/ bin/libintercept-new.so
	cp bin/libintercept-new.so bin/libintercept.so
	./chrome-experimenter.py --gui --verbose --direct-to-term --no-logs --interactive

summarize:
	-@echo "Using old summarize.sh script"
	./src/scripts/summarize.sh

logs/:
	mkdir logs/

bin/libintercept-old.so: bin/chrome_interpose.o bin/cpu_utils.o bin/experimenter-old.o bin/ipc.o
	clang++ -g -shared -fPIC -ldl -lg3logger -Wall $^ -o $@

bin/libintercept-new.so: bin/chrome_interpose.o bin/cpu_utils.o bin/experimenter-new.o bin/ipc.o
	clang++ -g -shared -fPIC -ldl -lg3logger -Wall $^ -o $@

bin/chrome_interpose.o: src/chrome_interpose.cc
	clang++ -c -g -Wall -fPIC $^ -o $@

bin/experimenter-old.o: src/experiment/experimenter.cc
	clang++ -c -g -Wall -fPIC -DRUNCHROME_MODE $^ -o $@

bin/experimenter-new.o: src/experiment/experimenter.cc
	clang++ -c -g -Wall -fPIC $^ -o $@

bin/ipc.o: src/experiment/ipc.cc
	clang++ -c -g -Wall -fPIC $^ -o $@

bin/cpu_utils.o: src/experiment/cpu_utils.cc
	clang++ -c -g -Wall -fPIC $^ -o $@

clean:
	-@rm bin/*.so bin/*.o *.out 2>/dev/null || true

clear-logs:
	-@rm logs/*.log 2>/dev/null || true
	-@rm -rf logs/exp-* 2>/dev/null || true
