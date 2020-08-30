PIDS="off"
VERBOSE="off"
CMDLINE_INFO="off"
RENDERERS_ONLY="off"
AFFINITY="off"
EXTRA_INFO="off"

echo_usage() {
    echo "usage: $0 [-hvpcra]"
    echo "-h : help,        shows this usage message"
    echo "-v : verbose,     output commands run. Default is off"
    echo "-c : cmdline,     show the command line args"
    echo "-r : renderers,   show only the renderer processes"
    echo "-r : refresh,     repeatedly output for a refreshing display"
    #echo "-b : browser,     show only the main browser process"
    #echo "-g : gpu,     show only the gpu process"
    #echo "-u : utility,     show only the utility process"
    echo "-a : affinity,    show process affinity"
    exit 0
}

while getopts ":hvcraF" opt; do
    case $opt in
        h)
            echo_usage
            ;;
        v)
            VERBOSE="on"
            ;;
        c)
            CMDLINE_INFO="on"
            EXTRA_INFO="on" # displaying more than just pid
            ;;
        r)
            RENDERERS_ONLY="on"
            ;;
        a)
            AFFINITY="on"
            EXTRA_INFO="on"
            ;;
        F)
            CONTINUOUS_MODE="on"
            ;;

        \?) # invalid arg
            echo "$0: Invalid arg -'$OPTARG'" >&2
            echo_usage
            ;;

#        :) # missing arg
#            case $OPTARG in
#                d)
#                    echo "$0: missing directory for option '-d'" >&2
#                    echo_usage
#                    ;;
#           esac
#            ;;

    esac
done

run_main() {
    if [[ "$RENDERERS_ONLY" == "on" ]]; then
        echo -n "Only Renderers: 'on'"
    else
        echo -n "Only Renderers: 'off'"
    fi
    if [[ "$CMDLINE_INFO" == "on" ]]; then
        echo " Commandline Info: 'on'"
    else
        echo " Commandline Info: 'off'"
    fi

    PROCS=`pgrep -x chrome`
    if [ -z "$PROCS" ]; then
        echo "No processes"
    else
        for i in `pgrep -x chrome`; do
            STR="/proc/$i/cmdline"

            if [[ "$RENDERERS_ONLY" == "on" ]]; then
                cat $STR | grep renderer &> /dev/null
                if [ $? -ne 0 ]; then
                    continue
                fi
            fi

            if [[ "$EXTRA_INFO" == "on" ]]; then
                echo -ne "$i:\t"
            else
                echo -ne "$i"
            fi

            if [[ "$CMDLINE_INFO" == "on" ]]; then
                cat $STR
                echo ""
            fi

            if [[ "$AFFINITY" == "on" ]]; then
                cat "/proc/$i/status" | grep -i "cpus_allowed"
            fi

            echo ""
        done
    fi
}

toggle() {
    if [[ "$1" == "on" ]]; then
        echo "off"
    else
        echo "on"
    fi
}

clear_screen() {
    clear
}

cleanup() {
    clear
    exit
}
trap cleanup SIGINT

clear_screen

while [[ "$CONTINUOUS_MODE" == "on" ]]; do
    run_main
    read -t 0.5 -N 1 input
    clear_screen
    if [[ "$input" == "q" ]] || [[ "$input" == "Q" ]]; then
        exit
    elif [[ "$input" == "c" ]] || [[ "$input" == "C" ]]; then
        CMDLINE_INFO=`toggle "$CMDLINE_INFO"`
    elif [[ "$input" == "r" ]] || [[ "$input" == "R" ]]; then
        RENDERERS_ONLY=`toggle "$RENDERERS_ONLY"`
    fi
done

run_main
