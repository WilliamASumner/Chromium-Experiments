#!/bin/bash
find logs/ -size 0 | xargs rm # remove empty entries, sometimes it quits before they can log
