#!/bin/bash

[[ $# -eq 2 ]] || { echo "Need 2 arg, got $#"; exit 1; }

[[ -f $1 ]] || { echo "$1 must be an existing file"; exit 4; }

[[ -d $2 ]] || { echo "$2 must be a directory"; exit 2; }

dirname="$(realpath "$2")"
filesCount="$(find "$dirname" -mindepth 1 | wc -l)"
[[ $filesCount -eq 0 ]] || { echo "$2 must be an empty directory"; exit 3; }

participants="$(sed -E 's|^([a-zA-Z-]+)[ ]+([a-zA-Z-]+).*:.*$|\1 \2|' "$1" \
    | sort \
    | uniq)"

participantsCount="$(echo "$participants" | wc -l)"
 
paste -d';' <(echo "$participants") <(seq 1 "$participantsCount") > "$dirname/dict.txt"

while read -r fName lName num; do
    
    grep -E "^$fName[ ]+$lName.*:.*$" "$1" > "$dirname/$num.txt"

done < <(paste -d' ' <(echo "$participants") <(seq 1 "$participantsCount"))
