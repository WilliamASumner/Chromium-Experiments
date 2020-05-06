#!/bin/bash

if [ -z $1 ]; then
	"usage: ./$0 -p PID"
	exit 1
fi

if [[ "$1" != "-p" ]]; then
	echo "Unrecognized flag $1"
	exit 1
fi

if [[ "$2" ~= "[0-9]+" ]]; then
		cat /proc/24736/maps | awk '{print $1 " " $6}' | grep '\.so' 
else
	"Error: invalid PID $2"
	exit 1
fi
