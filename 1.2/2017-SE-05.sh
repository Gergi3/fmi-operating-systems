#!/bin/bash

[[ $# -eq 2 ]] || { echo "Wrong number of args, needed 2, got $#"; exit 1; }

[[ -d $1 ]] || { echo "$1 must be a directory"; exit 2; }

# to avoid ambiguity
dirname="$(realpath $1)"

find "$dirname" -mindepth 1 -maxdepth 1 -type f -regextype posix-extended -regex ".*/vmlinuz-[0-9]+\.[0-9]+\.[0-9]+-$2" \
    | xargs -I {} basename {} \
    | sort -Vr -t'-' -k2 \
    | head -n 1
