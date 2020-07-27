.PHONY=clean run run-verbose run-graphical clear-logs run-pychrome

SITE_PREFIX:=+site

default: libintercept.so

run: logs/ libintercept.so
	./permutate.sh -c './run-chrome.sh' -f '-w +url -v' -p '$(SITE_PREFIX)'

run-verbose: logs/ libintercept.so
	./permutate.sh -v -c './run-chrome.sh' -f '-w +url -vi' -p '$(SITE_PREFIX)'

run-graphical: logs/ libintercept.so
	./permutate.sh -c './run-chrome.sh' -f '-w +url -vg' -p '$(SITE_PREFIX)'

run-pychrome: logs/ libintercept.so
	./python3 chrome-experimenter.py


logs/:
	mkdir logs/

libintercept.so: chrome_intercept.cc experiment/cpu_utils.* experiment/experimenter.*
	clang++ -g -shared -fPIC -ldl -lg3logger -o libintercept.so chrome_intercept.cc experiment/cpu_utils.cc experiment/experimenter.cc

clean:
	-@rm *.so *.out 2>/dev/null || true

clear-logs:
	-@rm logs/*.log 2>/dev/null || true
