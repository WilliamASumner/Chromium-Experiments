#!/bin/bash

trap "trap - SIGTERM && kill -9 -$$" SIGINT SIGTERM # kill allchildren on end
declare -a tasks_arr

function is_url {
    regex='(https?|ftp|file)://[-A-Za-z0-9\+&@#/%?=~_|!:,.;]*[-A-Za-z0-9\+&@#/%=~_|]'
    if [[ $1 =~ $regex ]]
    then 
        echo 0
    else
        echo -1
    fi
}

function site_name {
    echo `egrep --color=never -o [a-zA-Z0-9-]*[a-zA-Z0-9]\.[a-z][a-z][a-z]?$ <<< "$1"`
}
if [ -z $SITES ] && [ -z $1 ]; then
    SITES="https://www.amazon.com"
elif [ ! -z $1 ]; then # read in site list
    SITES="$1"
fi
tasks_arr=($SITES)

if [ -z PREFIX ]; then
    PREFIX=""
fi

if [ -z $REC_DEPTH ]; then
    REC_DEPTH=2
fi

if [ -z $MAX_SIZE ]; then
    MAX_SIZE="250m"
fi

if [ -z $RANDOM_WAIT ]; then
    RANDOM_WAIT="--random-wait"
fi

if [ -z $ROBOTS ]; then
    ROBOTS="-e robots=off"
fi

if [ -z $USER_AGENT ]; then # simulate rockpro user agent
    USER_AGENT="Mozilla/5.0 (X11; Ubuntu; Linux aarch64; rv:72.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4036.0 Safari/537.36"
fi

if [ -z $NO_PARENT ]; then
    NO_PARENT=" "
fi

# Explanation:
# -E = appends proper file extensions
# -k = convert links
# -H = span hosts
# --page-requisites = gather page prereqs such as css and js
# -r = recurse through child links
# -lN = limit depth to N
# -QN  = limit total fetch size to N (can be m,k,g,etc.)
# -np  = no parent directory followed
# --random-wait = wait randomly do avoid site scraper detection (might not be needed)
# --user-agent = set user agent

for task in ${tasks_arr[@]}; do
    ret=`is_url $task`
    if [[ "$ret" == "0" ]]; then # a valid url
        if [ -z $PREFIX ]; then # no prefix yet
            PREFIX="$PWD"
        fi

        PREFIX="$PREFIX/`site_name $task`"
        wget -EkH --page-requisites -r -l$REC_DEPTH -Q$MAX_SIZE $NO_PARENT $RANDOM_WAIT $ROBOTS --user-agent='$USER_AGENT' --directory-prefix=$PREFIX $task
    else # a file containing a list of urls
        if [ -z $PREFIX ]; then # no prefix yet
            PREFIX="$PWD/out-`site_name $task`"
        fi

        OLDPREFIX=$PREFIX
        while read line; do
            PREFIX=$OLDPREFIX
            isUrlTask=`is_url $line`
            if [[ "$isUrlTask" == "0" ]]; then # normal url
                ./wget-script.sh $line
            else
                #echo -e "$$:$SHLVL: forking $line\n"
                echo ""
                export PREFIX="$PREFIX/out-$line" # set the new prefix
                ./wget-script.sh $line & # fork for every list of urls to be a little speedier
            fi
        done < "$task"
    fi
done

wait
