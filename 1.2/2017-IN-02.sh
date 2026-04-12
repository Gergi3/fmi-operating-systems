#!/bin/bash

[[ $# -eq 1 ]] || { echo "Wrong number of args"; exit 1; }

# Comment line below to test
#[[ $(whoami) == 'root' ]] || { echo "You must be logged in as root to run this"; exit 2; }

if ! ps -U $1 &>/dev/null; then
    echo "User does not exist"
    exit 2;
fi

# a)
process_counts_info="$(ps -e -o user \
    | tail -n +2 \
    | sort \
    | uniq -c \
    | sort -nr )"

echo "$process_counts_info"
process_counts="$(echo "$process_counts_info" | awk 'BEGIN {sum = 0} {sum += $1} END {print sum}')"

# b)

all_time="$(ps -e -o time \
    | tail -n +2 \
    | awk -F':' 'BEGIN { time=0 } { time += $1 * 3600 + $2 * 60 + $1 } END {print time}')"

avg_time="$(echo "$all_time / $process_counts" | bc -l)"
echo "Average time of all processes: $avg_time"

# c)
ps -e -o pid,time \
    | tail -n +2 \
    | awk '{$1=$1} {print $0}' \
    | awk -F'[: ]' -v avg_time="$avg_time" '(avg_time * 2) < ($2 * 3600 + $3 * 60 + $4) {print $1}' \
    | xargs kill -SIGKILL
