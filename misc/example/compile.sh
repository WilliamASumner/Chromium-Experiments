#!/bin/bash
### Example
clang++ -Wall -g -fPIC -shared -ldl -o liboriginal.so HelloPrinter.cc # original library example
clang++ -Wall -g -fPIC -shared -o intercept.so hello_intercept.cc # interposing library example
clang++ -Wall -g main.cc -L. -loriginal -o prog.out # main program example
