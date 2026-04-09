#!/bin/bash

# comment to test
[[ $(whoami) == "root" ]] || { echo "You must be logged in as root!"; exit 1; }


info="$(ps -e -o uname,rss,pid \
    | tail -n +2 \
    | awk '
        $2 > maxPidRss[$1] {maxPidRss[$1] = $2; maxPid[$1] = $3}
        {cnt[$1] += 1} 
        {sum[$1] += $2}
        END {for (name in cnt) { print name, cnt[name], sum[name], maxPid[name], maxPidRss[name] } }')"

echo -e "NAME PROCESS_COUNT RSS_SUM MAX_PID MAX_PID_RSS\n$info" \
    | cut -d' ' -f1-3 \
    | column -t

while read -r name processCount rssSum maxPid maxPidRss; do

    [[ $processCount -eq 0 ]] && continue
    
    isBigger="$(echo "($rssSum / $processCount) > $maxPidRss" | bc -l)"
    
    if [[ $isBigger -eq 1 ]]; then
        kill -SIGKILL "$maxPid"

        # uncomment to test
        # echo "kill -SIGKILL \"$maxPid\""
    fi
done < <(echo "$info")
