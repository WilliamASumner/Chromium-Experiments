#!/bin/bash

FILE_PREFIX="output"
ITERATIONS=10

declare -a configs governors iterconfig itergovernor sites # declare arrays
configs=("4l-0b" "4l-4b" "0l-4b" "4l-1b" "4l-2b" "2l-0b" "1l-0b" "0l-1b" "0l-2b") # all core configs to test with
governors=("pi" "ii" "ip" "pp") # all core configs to test with
sites=("cnn.com")

array_contains () {
    local array="$1[@]"
    local seeking=$2
    local in=1
    for element in "${!array}"; do
        if [[ $element == $seeking ]]; then
            in=0
            break
        fi
    done
    return $in
}

gen_iterconfig() { # generate a permutation of the configs
    iterconfig=()
    for (( i=1; i <= 7; i++ ))
    do
        x=$( echo "$RANDOM % 7" | bc )
        array_contains iterconfig ${configs["$x"]}
        while [ $? -eq 0 ]; do
            x=$( echo "$RANDOM % 7" | bc )
            array_contains iterconfig ${configs["$x"]}
        done
        iterconfig=("${iterconfig[@]}" ${configs["$x"]})
    done
}

gen_itergovernor() { # generate a permutation of the governors
    itergovernor=()
    for (( i=1; i <= 4; i++ ))
    do
        x=$( echo "$RANDOM % 4" | bc )
        array_contains itergovernor ${governors["$x"]}
        while [ $? -eq 0 ]; do
            x=$( echo "$RANDOM % 4" | bc )
            array_contains itergovernor ${governors["$x"]}
        done
        itergovernor=("${itergovernor[@]}" ${governors["$x"]})
    done
}


echo_usage() {
    echo "usage: $0 [-h | -f PREFIX | -i ITERATIONS | -c COMMAND]"
    echo "-h : Help,       shows this usage message"
    echo "-f : Prefix,     the prefix to use for files generated from the script. Useful for labelling experiment trials"
    echo "-i : Iterations, number of iterations to run"
    echo "-c : Command,    command to be run on each permutation"
    exit 0
}

while getopts ":hi:f:c:" opt; do
    case $opt in
        h)
            echo_usage
            ;;

        f)
            FILE_PREFIX=$OPTARG
            ;;

        i)
            ITERATIONS=$OPTARG
            ;;
        c)
            COMMAND=$OPTARG
            ;;
        \?) # invalid arg
            echo "Invalid arg -$OPTARG" >&2
            echo_usage
            ;;

        :) # missing arg
            case $OPTARG in
                f)
                    echo "$0: missing prefix for option '-f'" >&2
                    echo_usage
                    ;;
                i)
                    echo "$0: missing iterations argument for option '-i'"
                    echo_usage
                    ;;
                c)
                    echo "$0: missing command argument for option '-c'"
                    echo_usage
                    ;;
            esac
            ;;

    esac
done

for (( iter=1; iter <=$ITERATIONS; iter++ )); do
    echo "iteration $iter"

    # for each governor, could be for each config first,
    # but the idea behind looping through various configs 
    # in the inner loop is to possiblity that caching will help a config
    # since it will not be run consecutively 
    #gen_itergovernor
#   for gov in "ii"; do #${itergovernor[@]}; do  # for each governor, configs could be looped through first,
        gen_iterconfig
        for config in ${iterconfig[@]}; do
            for site in ${sites[@]}; do
                ID=`mktemp -u XXXXXXXX` # unique experiment id
                export CORE_CONFIG=$config
                export LOG_FILE="$FILE_PREFIX-$ID"
                echo "on permutation $config $FILE_PREFIX-$ID $site"
                #export GOVERNOR=$gov # not needed right now
                $COMMAND
                #./run.sh $config $FILE_PREFIX $site
                RETVAL=$?
                if [[ "$RETVAL" == "1" ]]; then # script didn't like something...
                    echo "script exited with an error, see above output"
                    exit
                elif [[ "$RETVAL" == "2" ]]; then # script was hit with a ctrl-c
                    echo  "script caught a SIGINT, exiting..."
                    exit
                fi
            done # site
        done # config
    #done #gov
done
