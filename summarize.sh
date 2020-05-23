#!/bin/bash

FUNC_LOG="func_latencies.log"
SUMMARY_LOG="summary.log"
EMPTY_FILES=`find logs/ -size 0`
if [[ "$EMPTY_FILES" != "" ]]; then
    find logs/ -size 0 | xargs rm || true # remove empty entries, some processes quit before they can log
fi
cd logs
echo -n "" > $FUNC_LOG

for file in `ls -1`; do
    if [[ "$file" == "format.md" ]]; then
        continue
    elif [ -d $file ]; then
        continue
    fi
    # Generate function latency list
    cat $file | tail +4 | awk '{ if (NF == 9  && $4=="INFO") print $7"\t"$9 }' >> $FUNC_LOG
    cat $file | tail +4 | awk '{ if (NF == 8  && $4=="INFO" && $7=="PageLoadTime") print $7"\t"$8 }' >> $FUNC_LOG
done

cat $FUNC_LOG | sort | uniq > $SUMMARY_LOG

cat <(echo -e "Function\tLatency(ms)") $SUMMARY_LOG > /tmp/$SUMMARY_LOG # add a header
cp /tmp/$SUMMARY_LOG ./$SUMMARY_LOG

# Datamash important flags
# -H = use file headers
# -s = sort data
# -g x op y = group col x by op on col(y), see below

# Datamash util examples
# datamash -H -s -g 1 count 2 < logs/summary.log # group function names by num. executions
# datamash -H -s -g 1 median 2 < logs/summary.log # group function names by median of latencies
datamash -H -s -g 1 median 2 mean 2 count 2 q1 2 q3 2 < $SUMMARY_LOG > /tmp/$SUMMARY_LOG
cat /tmp/$SUMMARY_LOG | column -t > ./$SUMMARY_LOG
