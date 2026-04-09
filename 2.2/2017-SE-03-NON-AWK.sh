#!/bin/bash

# comment to test
[[ $(whoami) == "root" ]] || { echo "You must be logged in as root!"; exit 1; }

declare -A cnt
declare -A sum
declare -A maxPid
declare -A maxPidRss

while read -r uname rss pid; do
    (( sum[$uname] += $rss ))
    (( cnt[$uname]++ ))

    if [[ ${maxPidRss[$uname]} -lt $rss ]]; then
        maxPidRss[$uname]=$rss
        maxPid[$uname]=$pid
    fi
done < <(ps -e -o uname,rss,pid | tail -n +2)

info=$(mktemp)
echo "NAME PROCESS_COUNT RSS_SUM MAX_PID MAX_PID_RSS\n" >> "$info"
for key in "${!cnt[@]}"; do
    echo "$key ${cnt[$key]} ${sum[$key]} ${maxPid[$key]} ${maxPidRss[$key]}\n" >> "$info"
    
    isBigger="$(echo "${sum[$key]} / ${cnt[$key]} > ${maxPidRss[$key]}" | bc -l)"
    
    if [[ $isBigger -eq 1 ]]; then
        kill -SIGKILL "${maxPid[$key]}"
    fi
done

cat "$info" \
    | cut -d' ' -f1-3 \
    | column -t

rm $info
