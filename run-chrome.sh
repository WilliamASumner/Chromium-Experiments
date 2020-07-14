#!/bin/bash

LD_PRELOAD_VAL=$PWD/libintercept.so
export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
XVFB_PID=`pgrep Xvfb`
BINARY=chrome
CHROME_DIR=/home/vagrant/chromium/src/out/x64Linux
WEBPAGE="cnn.com"
FLAGS="--no-zygote --no-sandbox"

if [ -z $VERBOSE ]; then
    VERBOSE=0
fi
CHROME_VERBOSE=0

echo_usage() {
    echo "usage: $0 [-hpgv |-b BIN | -d DIR |-l {LD_PRELOAD_PATH|None} |-w www.example.com |-f 'Flags' ]"
    echo "-h : help,        shows this usage message"
    echo "-p : pause,       adds a value to Chromium flags to pause for gdb"
    echo "-g : gui,         runs Chromium with a gui rather than Xvfb"
    echo "-v : verbose,     output commands run. Default is off"
    echo "-i : info,        adds a flag to Chromium flags which increases log verbosity."
    echo "-b : [binary],    changes binary to specified file. Default is '$BINARY'"
    echo "-d : [dir],       changes directory where specified binary is located. Default is '$CHROME_DIR'"
    echo "-l : [ldpreload], changes LD_PRELOAD value. 'None' unsets LD_PRELOAD. Default is '\$PWD/${LD_PRELOAD_VAL##*/}'"
    echo "-w : [webpage],   changes default page to be loaded. Default is '$WEBPAGE'"
    echo "-f : [flags],     changes Chromium flags. Default is '$FLAGS'. Prefix your flags with 'clear:' to completely override."
    echo "                  E.g. 'clear: --no-sandbox' will create the command '$CHROME_DIR/$BINARY --no-sandbox $WEBPAGE'"
    exit 0
}

while getopts ":hpgvib:d:l:w:f:" opt; do
    case $opt in
        h)
            echo_usage
            ;;

        p)
            FLAGS="$FLAGS --renderer-startup-dialog"
            ;;

        g)
            XVFB_PID=-1
            ;;
        v)
            VERBOSE=1
            ;;
        i)
            FLAGS="$FLAGS --enable--logging=stderr --v=1"
            CHROME_VERBOSE=1
            ;;

        b)
            BINARY="$OPTARG"
            ;;

        d)
            if [ ! -d "$OPTARG" ]; then
                echo "$0: invalid directory '$OPTARG' for option '-l'"
                exit 1
            fi
            CHROME_DIR="$OPTARG"
            ;;

        l)
            if [[ "$OPTARG" == "None" ]]; then
                echo "Received 'None' for option '-l', unsetting LD_PRELOAD"
                unset LD_PRELOAD_VAL
                unset LD_PRELOAD
            elif [ ! -f "$OPTARG" ]; then
                echo "$0: invalid preload file '$OPTARG' for option '-l'"
                exit 1
            else
                LD_PRELOAD_VAL="$OPTARG"
            fi
            ;;

        w)
            WEBPAGE="$OPTARG"
            ;;

        f)
            if [[ "${OPTARGS%:}" == "clear" ]]; then
                FLAGS="$OPTARG"
            else
                FLAGS="$FLAGS $OPTARG"
            fi
            ;;

        \?) # invalid arg
            echo "Invalid arg -$OPTARG" >&2
            echo_usage
            ;;

        :) # missing arg
            case $OPTARG in
                d)
                    echo "$0: missing directory for option '-d'" >&2
                    echo_usage
                    ;;
                l)
                    echo "$0: missing argument for option '-l', use 'None' to prevent preloading"
                    echo_usage
                    ;;
                w)
                    echo "$0: missing webpage for option '-w'" >&2
                    echo_usage
                    ;;
                f)
                    echo "$0: missing quoted flags string for option '-f'" >&2
                    echo_usage
                    ;;
            esac
            ;;

    esac
done

if [ -z $XVFB_PID ]; then
    if [[ "$VERBOSE" == 1 ]]; then 
        echo "Starting Xvfb"
    fi
    Xvfb :99 -screen 0 800x600x16 &
    export DISPLAY=:99
elif ! [[ "$XVFB_PID" == "-1" ]]; then
    if [[ "$VERBOSE" == 1 ]]; then 
        echo "Connecting to existing Xvfb"
    fi
    export DISPLAY=:99
fi

if [[ "$VERBOSE" == "1" ]]; then
    echo "LD_PRELOAD: $LD_PRELOAD_VAL"
    echo "$CHROME_DIR/$BINARY $FLAGS $WEBPAGE"
fi

# Since this affects all commands, export here so we don't affect other cmds
if [ ! -z $LD_PRELOAD_VAL ]; then 
    export LD_PRELOAD=$LD_PRELOAD_VAL
fi

if [[ "$CHROME_VERBOSE" == "1" ]]; then
    $CHROME_DIR/$BINARY $FLAGS $WEBPAGE > stdout_log.txt 2> stderr_log.txt
else
    $CHROME_DIR/$BINARY $FLAGS $WEBPAGE
fi

