#!/bin/bash
#Chrome script
clang++ -shared -fPIC -ldl -o intercept-chrome.so chrome_intercept.cc 
