#!/bin/bash

export EXP_DIR="$PWD"

# Run occupancy processing
export DATA_FILES=`ls $EXP_DIR/logs/example_data/*.log`
export OUT_FILE="$PWD/plotting/occupancy-graph.png"
python3 ./plotting/occupancy.py
