#!/bin/bash

FUNC_LOG="func_latencies.log"
SUMMARY_LOG="summary.log"
if [ -z $EXP_DATALOG_DIR ]; then
    EXP_DATALOG_DIR="$PWD/logs/"
fi
EMPTY_FILES=`find $EXP_DATALOG_DIR -size 0`
if [[ "$EMPTY_FILES" != "" ]]; then
    find $EXP_DATALOG_DIR -size 0 | xargs rm || true # remove empty entries, some processes quit before they can log
fi

cd $EXP_DATALOG_DIR
rm $SUMMARY_LOG 2>/dev/null
rm $FUNC_LOG 2>/dev/null
ls *.log &>/dev/null
if [ $? -ne 0 ]; then
    echo "Error: No logs files found."
    exit 1
fi
echo -n "" > $FUNC_LOG

# Generate function latency list
for file in `ls -1`; do
    if [[ "$file" == "format.md" ]] || [[ "$file" == "$SUMMARY_LOG" ]] || [[ "$file" == "$FUNC_LOG" ]]; then
        continue
    elif [ -d $file ]; then
        continue
    fi

    cat $file | tail +4 | awk '{ if (NF == 10  && $4=="INFO") print $7"\t"$10 }' >> $FUNC_LOG # latency
    cat $file | tail +4 | awk '{ if (NF == 8  && $4=="INFO" && $7=="PageLoadTime") print $7"\t"$8 }' >> $FUNC_LOG # page load time
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
datamash -H -s -g 1 median 2 mean 2 q1 2 q3 2 count 2 min 2 max 2 < $SUMMARY_LOG > /tmp/$SUMMARY_LOG
cat /tmp/$SUMMARY_LOG | column -t > ./$SUMMARY_LOG
cp ./$SUMMARY_LOG /tmp/$SUMMARY_LOG

awk '{ if($1 != "PageLoadTime") { print }}' /tmp/$SUMMARY_LOG > ./$SUMMARY_LOG # reorder PageLoadTime to the bottom
echo "-------------------------" >> ./$SUMMARY_LOG
awk '{ if($1 == "PageLoadTime") { print }}' /tmp/$SUMMARY_LOG >> ./$SUMMARY_LOG
