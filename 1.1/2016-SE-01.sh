#!/bin/bash

[[ $# -eq 1 ]] || { echo "You need to provide a filename" >&2; exit 1; }

filename="$1"
[[ -f $filename ]] || { echo "File $filename does not exist" >&2; exit 2; }

grep -vE '[a-w]' -- $filename | grep -cE '[02468]'
