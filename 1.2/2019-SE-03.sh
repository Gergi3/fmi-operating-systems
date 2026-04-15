#!/bin/bash

[[ $# -eq 1 ]] || { echo "1 arg required!"; exit 1; }

[[ -d $1 ]] || { echo "$1 must be a directory!"; exit 2; }

dirname="$(realpath $1)"

mkdir extracted 2>/dev/null
while read -r file; do 
    new_name="$(echo "$file" | sed -E 's#^.*/([^_]+)_report-([0-9]+).tgz#extracted/\1_\2.txt#')"
    
    tar -O --wildcards -x '*/meow.txt' -f "$file" 2>/dev/null > $new_name

done < <(find $dirname -regextype posix-extended -regex '^[^_]+_report-[0-9]+.tgz' 2>/dev/null)

