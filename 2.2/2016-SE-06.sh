#!/bin/bash

[[ $# -eq 1 ]] || { echo "Invalid number of args"; exit 1; }
[[ -f $1 ]] || { echo "Arg must be a file"; exit 2; }

awk -F' - ' 'BEGIN {cnt=1} {print cnt ". " $2 " - " $3; cnt++}' $1 | sort -t '.' -k2

