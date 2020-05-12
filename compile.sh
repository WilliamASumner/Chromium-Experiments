#!/bin/bash
#Chrome script
clang++ -g -shared -fPIC -ldl -o intercept-chrome.so chrome_intercept.cc 
