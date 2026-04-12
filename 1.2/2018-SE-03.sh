#!/bin/bash

[[ $# -eq 2 ]] || { echo "2 args required, got $#"; exit 1; }

[[ -f $1 ]] || { echo "$1 must be an existing file"; exit 2; }

# same awk logic can be done with bash constructs and bash associative array
cat "$1" \
    | sort -t',' -k2 -k1,1n \
    | awk -F ',' '{
        k = ""
        for (i = 2; i <= NF; i++) k = k "," $i
        if (!seen[k]++) { print $0 } }' > "$2"

