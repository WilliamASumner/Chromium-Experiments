#!/bin/bash

FUNC_LOG="func_latencies.log"
SUMMARY_LOG="summary.log"

find logs/ -size 0 | xargs rm || true # remove empty entries, some processes quit before they can log
cd logs
echo -n "" > $FUNC_LOG

for file in `ls -1`; do
    if [[ "$file" == "format.md" ]]; then
        continue
    fi
    # Generate function latency list
    cat $file | tail +4 | awk '{ print $7"\t"$9 }' >> $FUNC_LOG
done

cat $FUNC_LOG | sort | uniq > $SUMMARY_LOG
