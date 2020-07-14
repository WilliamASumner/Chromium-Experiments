#!/bin/bash

FILE_PREFIX="output"
ITERATIONS=1
COMMAND="echo"
FLAGS=""

declare -a configs governors iterconfig itergovernor sites # declare arrays
#configs=("4l-0b" "4l-4b" "0l-4b" "4l-1b" "4l-2b" "2l-0b" "1l-0b" "0l-1b" "0l-2b") # all core configs to test with
configs=("4l-0b") # all core configs to test with
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
    while [[ "${#iterconfig[@]}" != "${#configs[@]}" ]]
    do
        x=$( echo "$RANDOM % ${#configs[@]}" | bc )
        array_contains iterconfig ${configs["$x"]}
        while [ $? -eq 0 ]; do
            x=$( echo "$RANDOM % ${#configs[@]}" | bc )
            array_contains iterconfig ${configs["$x"]}
        done
        iterconfig=("${iterconfig[@]}" ${configs["$x"]})
    done
}

gen_itergovernor() { # generate a permutation of the governors
    itergovernor=()
    while [[ "${#itergovernor[@]}" != "${#governors[@]}" ]]
    do
        x=$( echo "$RANDOM % ${#governors[@]}" | bc )
        array_contains itergovernor ${governors["$x"]}
        while [ $? -eq 0 ]; do
            x=$( echo "$RANDOM % ${#governors[@]}" | bc )
            array_contains itergovernor ${governors["$x"]}
        done
        itergovernor=("${itergovernor[@]}" ${governors["$x"]})
    done
}

gen_itersite() { # generate a permutation of websites
    itersite=()
    while [[ "${#itersite[@]}" != "${#sites[@]}" ]]
    do
        x=$( echo "$RANDOM % ${#sites[@]}" | bc )
        array_contains itersite ${sites["$x"]}
        while [ $? -eq 0 ]; do
            x=$( echo "$RANDOM % ${#sites[@]}" | bc )
            array_contains itersite ${sites["$x"]}
        done
        itersite=("${itersite[@]}" ${sites["$x"]})
    done
}

expand_FLAGS_and_FP() {
    FLAGS=${FLAGS//'+config'/"$config"}
    FLAGS=${FLAGS//'+site'/"$SITENAME"}
    FLAGS=${FLAGS//'+url'/"$url"}
    FLAGS=${FLAGS//'+iter'/"$iter"}
    FLAGS=${FLAGS//'+gov'/"$gov"}

    FILE_PREFIX_P=${FILE_PREFIX//'+config'/"$config"}
    FILE_PREFIX_P=${FILE_PREFIX_P//'+site'/"$SITENAME"}
    FILE_PREFIX_P=${FILE_PREFIX_P//'+url'/"$url"}
    FILE_PREFIX_P=${FILE_PREFIX_P//'+iter'/"$iter"}
    FILE_PREFIX_P=${FILE_PREFIX_P//'+gov'/"$gov"}
}



echo_usage() {
    echo "usage: $0 [-h | -p PREFIX | -i ITERATIONS | -c COMMAND | -f FLAGS | -v]"
    echo "-h : Help,       shows this usage message"
    echo "-p : Prefix,     the prefix to use for files generated from the script. Useful for labeling experiment trials"
    echo "-i : Iterations, number of iterations to run"
    echo "-c : Command,    command to be run on each permutation"
    echo "-f : Flags,      flags to be run with the given command"
    echo "-v : Verbose,    run with verbose output"
    echo "Predefined values: +url = url, +site = website, +gov = freq gov, +iter = iteration, +config = core configuration"
    echo "Note: the site keyword returns a trimmed value of the url: 'http://www.example.com' -> 'example' for compactness"
    exit 0
}

while getopts ":hp:i:c:f:v" opt; do
    case $opt in
        h)
            echo_usage
            ;;

        p)
            FILE_PREFIX=$OPTARG
            ;;

        i)
            ITERATIONS=$OPTARG
            ;;
        c)
            COMMAND=$OPTARG
            ;;

        f)
            FLAGS=$OPTARG
            ;;

        v)  export VERBOSE=1
            ;;

        \?) # invalid arg
            echo "Invalid arg -$OPTARG" >&2
            echo_usage
            ;;

        :) # missing arg
            case $OPTARG in
                p)
                    echo "$0: missing prefix for option '-p'" >&2
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
                f)
                    echo "$0: missing flags argument for option '-f'"
                    echo_usage
                    ;;
            esac
            ;;

    esac
done


for (( iter=1; iter <=$ITERATIONS; iter++ )); do

    if [[ "$VERBOSE" == "1" ]]; then 
        echo "iteration $iter"
    fi

    # for each governor, could be for each config first,
    # but the idea behind looping through various configs 
    # in the inner loop is to possiblity that caching will help a config
    # since it will not be run consecutively 
    #gen_itergovernor
#   for gov in "ii"; do #${itergovernor[@]}; do  # for each governor, configs could be looped through first,
        gen_iterconfig
        for config in ${iterconfig[@]}; do
            gen_itersite
            for url in ${itersite[@]}; do
                ID=`mktemp -u XXXXXXXX` # unique experiment id
                SITENAME="${url%.*}" # remove ".com", ".edu" etc
                SITENAME="${SITENAME#*www.}" # remove "http://www."

                expand_FLAGS_and_FP # expand embedded variables

                if [[ "$VERBOSE" == "1" ]]; then 
                    echo "on permutation CORE_CONFIG=$config LOG_FILE=$FILE_PREFIX_P-$ID for site '$SITENAME'"
                    echo "'$COMMAND' $FLAGS"
                fi

                #export GOVERNOR=$gov # not needed right now
                export CORE_CONFIG=$config
                export LOG_FILE="$FILE_PREFIX_P-$ID"
                $COMMAND "$FLAGS"
                RETVAL=$?

                if [[ "$RETVAL" == "1" ]]; then # script didn't like something...
                    echo "script exited with an error, see above output"
                    exit
                elif [[ "$RETVAL" == "2" ]]; then # script was hit with a ctrl-c
                    echo  "script caught a SIGINT, exiting..."
                    exit
                fi
            done # url
        done # config
    #done #gov
done
