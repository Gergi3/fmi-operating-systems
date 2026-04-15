#!/bin/bash

[[ $# -eq 3 ]] || { echo "Three args required: num prefix_symbol unit_symbol" >&2; exit 1; }

[[ $1 =~ [0-9]+ ]] || { echo "$1 must be a number" >&2; exit 2; }

[[ -n $2 ]] || { echo "Prefix symbol cannot be empty" >&2; exit 3; }

[[ -n $3 ]] || { echo "Unit symbol must be nonempty" >&2; exit 4; }

base_f="./base.csv"
prefix_f="./prefix.csv"

num="$1"
prefix_s="$2"
unit_s="$3"

# 2.1 M s
# 2100000.0 s (time, second)

mult="$(awk -F',' -v prefix_s="$prefix_s" '$2 == prefix_s { print $3 }' "$prefix_f")"

if [[ $(echo "$mult" | wc -l) -ne 1 ]]; then
    echo "Ambigous prefix symbol ($prefix_s is seen more than once in $prefix_f)" >&2;
    exit 5;
fi

units="$(awk -F',' -v unit_s="$unit_s" '$2 == unit_s { print "(" $3 ", " $1 ")" }' "$base_f")"

if [[ $(echo "$units" | wc -l) -ne 1 ]]; then
    echo "Ambigous units symbol ($unit_s is seen more than once in $base_f)" >&2;
    exit 6;
fi

new_num="$(echo "$mult * $num" | bc -l)"

echo "$new_num $unit_s $units"

