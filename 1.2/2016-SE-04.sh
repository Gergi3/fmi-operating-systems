#!/bin/bash

# ERROR CODES
# 1 - wrong num of params
# 2 - wrong param format

dirname=$PWD

[[ $# -eq 2 ]] || { echo "Wrong number of params!"; exit 1; }

[[ $1 =~ [0-9]+ && $2 =~ [0-9]+ ]] || { echo "Params must be numbers!"; exit 2; }

[[ -d 'a' ]] || { mkdir a; }
[[ -d 'b' ]] || { mkdir b; }
[[ -d 'c' ]] || { mkdir c; }


files="$(find . -mindepth 1 -maxdepth 1 -type f)"

while read file; do
    [[ -f $file ]] || continue

    lines="$(wc -l "$file" | xargs | cut -d' ' -f1)"
    if [[ $lines -lt $1 ]]; then
        mv "$file" a 
    elif [[ $lines -le $2 ]]; then
        mv "$file" b
    else
        mv "$file" c
    fi

done < <(echo "$files")

