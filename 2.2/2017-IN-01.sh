#!/bin/bash

[[ $# -eq 3 ]] || { echo "Wrong number of args"; exit 1; }
[[ -f $1 ]] || { echo "$1 must be an existing regular file"; exit 2; }
[[ -n $2 && -n $3 ]] || { echo "Args cannot be empty strings"; exit 3; }

file="$1"
k1="$2"
k2="$3"

v1="$(grep -E "^$k1=.*$" $file | cut -d'=' -f2 | tr ' ' '\n')"
v2="$(grep -E "^$k2=.*$" $file | cut -d'=' -f2 | tr ' ' '\n')"

newV2="$(comm <(echo "$v1" | sort) <(echo "$v2" | sort) -13 | tr '\n' ' ')"

result="$(awk -F'=' -v k2="$k2" -v newV2="$newV2" '
    { 
        if ($1 == k2) { print $1 "=" newV2 }
        else { print $0 }
    }' "$file")"

echo "$result" > $file

