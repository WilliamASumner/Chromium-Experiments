#!/bin/bash

export LD_LIBRARY_PATH="$PWD:$LD_LIBRARY_PATH"
if [[ "$1" == "no_pre" ]]; then
	unset LD_PRELOAD
else
	export LD_PRELOAD="$PWD/intercept.so"
fi
echo "run.sh: Preloading... $LD_PRELOAD"
echo ""
./prog.out
