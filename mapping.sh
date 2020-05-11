#!/bin/bash

PID=-1
while getopts ":hp:" opt; do
    case $opt in
        p)
            if ! [[ "$OPTARG" =~ ^[0-9]+$ ]]; then
                echo "Invalid PID $OPTARG" >&2
                exit 1
            elif [ ! -d "/proc/$OPTARG" ]; then
                echo "PID $OPTARG not found" >&2
                exit 1
            else
                PID="$OPTARG"
            fi
            ;;
        h)
            echo "usage: $0 -p PID"
            echo "This script outputs the mappings of libraries of a process"
            exit 1
            ;;
        \?) # invalid arg
            echo "Invalid option: -$OPTARG" >&2
            exit 1
            ;;
        :) # missing arg
            case $OPTARG in
                p)
                    echo "-p requires a PID" >&2
                    exit 1
                    ;;
            esac
    esac
done

if [[ "$PID" == "-1" ]]; then
    echo "Error: missing '-p'" >&2
    exit 1
fi

cat /proc/$PID/maps | awk '{print $1 " " $6}' | grep '\.so'
