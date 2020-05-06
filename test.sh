#!/bin/bash

XVFB_PID=`pgrep Xvfb`
if [ -z $XVFB_PID ]; then # no Xvfb yet, start it
    echo "Starting Xvfb"
    Xvfb :99 &
fi
export DISPLAY=:99



SITE="cnn.com"
CHROME_DIR=/home/vagrant/chromium/src/out/chrome-coz
FLAGS="--no-zygote --no-sandbox"

if [[ "$1" == "nopre" ]]; then
	unset LD_PRELOAD
elif [[ "$1" != "" ]]; then 
	SITE="$1"
else
	export LD_PRELOAD="$PWD/intercept-chrome.so"
fi
$CHROME_DIR/chrome $FLAGS "$SITE"
