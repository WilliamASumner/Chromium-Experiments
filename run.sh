#!/bin/bash

export LD_PRELOAD=$PWD/libintercept.so
XVFB_PID=`pgrep Xvfb`
BINARY=chrome
CHROME_DIR=/home/vagrant/chromium/src/out/x64Linux
WEBPAGE="cnn.com"
FLAGS="--no-zygote --no-sandbox"

echo_usage() {
    echo "usage: $0 [-hpg |-b BIN | -d DIR |-l {LD_PRELOAD_PATH|None} |-w www.example.com |-f 'Flags' ]"
    echo "-h : Help,      shows this usage message"
    echo "-p : Pause,     adds a flag to Chromium flags to pause for gdb"
    echo "-g : Gui,       runs Chromium with a gui rather than Xvfb"
    echo "-b : Binary,    changes binary to specified file. Default is 'chrome'"
    echo "-d : Dir,       changes directory where specified binary is located"
    echo "-l : Ldpreload, changes LD_PRELOAD value. No value unsets LD_PRELOAD. Default is './libintercept.so'"
    echo "-w : Webpage,   changes default page to be loaded. Default is 'cnn.com'"
    echo "-f : Flags,     changes Chromium flags. Default is '--no-zyogote --no-sandbox'"
    exit 0
}

while getopts ":hpgb:d:l:w:f:" opt; do
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
            if [[ "$f" == "None" ]]; then
                echo "Received 'None' for option '-l', unsetting LD_PRELOAD"
                unset LD_PRELOAD
            elif [ ! -f "$OPTARG" ]; then
                echo "$0: invalid preload file '$OPTARG' for option '-l'"
                exit 1
            fi

            LD_PRELOAD="$OPTARG"
            ;;

        w)
            WEBPAGE="$OPTARG"
            ;;

        f)
            FLAGS="$OPTARG"
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
    echo "Starting Xvfb"
    Xvfb :99 -screen 0 800x600x16 &
    export DISPLAY=:99
elif ! [[ "$XVFB_PID" == "-1" ]]; then
    echo "Connecting to existing Xvfb"
    export DISPLAY=:99
fi

echo "$CHROME_DIR/$BINARY $FLAGS $WEBPAGE"
$CHROME_DIR/$BINARY $FLAGS $WEBPAGE
