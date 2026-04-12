#!/bin/bash

[[ $# -eq 1 ]] || { echo "Invalid number of args, must be 1, got $#"; exit 1; }
[[ -d $1 ]] || { echo "$1 must be a directory!"; exit 2; }

# to avoid ambiguity
logdir="$(realpath $1)"

find "$logdir" -mindepth 4 -maxdepth 4 \
    | xargs -I {} wc -l {} \
    | awk '{$1=$1} {print $0}' \
    | sed -E 's|([0-9]+) /.*/(.*)/[0-9]{4}-[0-9]{2}-[0-9]{2}-[0-9]{2}-[0-9]{2}-[0-9]{2}\.txt|\1 \2|' \
    | awk '{fMap[$2]+=$1} END { for (fName in fMap) { print fMap[fName], fName }}' \
    | sort -nr -k1 \
    | head -n 10
