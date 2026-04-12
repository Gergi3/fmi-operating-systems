#!/bin/bash

# ERROR CODES
# 1 - wrong number of args
# 2 - must be files and exist

[[ $# -eq 2 ]] || { echo "Wrong number of args"; exit 1; }

[[ -f $1 && -f $2 ]] || { echo "Args must be existing files"; exit 2; }

winner=$1

if [[ $(grep -cE "$2" "$2") -gt $(grep -cE "$1" "$1" ) ]] then
    winner=$2
fi

awk -F'-' '{print $2}' $winner | awk '{$1=$1} {print $0}' |sort > "$winner.songs" 
