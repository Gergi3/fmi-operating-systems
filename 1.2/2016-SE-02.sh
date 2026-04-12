#!/bin/bash

# ERROR CODES
# 1 - INVALID ARG COUNT
# 2 - INVALID ARG FORMAT
# 3 - WRONG USER

NEEDED_ARGS=1
[[ $# -eq $NEEDED_ARGS ]] || { echo "Invalid arguments count (expected: $NEEDED_ARGS, got: $#)" >&2; exit 1; }

MAX_RSS=$1
[[ $MAX_RSS =~ ^[0-9]+$ ]] || { echo "Argument must be a number but was: $MAX_RSS" >&2; exit 2; }

# to test
#USER_REQUIRED=$(whoami)

USER_REQUIRED="root"
[[ $(whoami) = $USER_REQUIRED ]] || { echo "You must be $USER_REQUIRED to continue" >&2; exit 3; }

data="$(ps -e -o uname,rss | tail -n +2 | sort -k1 | awk '
    BEGIN { print "USERNAME RSS" } 
    { nameSumMap[$1] += $2 } 
    END { for (name in nameSumMap) { print name, nameSumMap[name] } }')"

echo "$data"

while read -r uname rssSum; do
    if [[ $rssSum -gt $MAX_RSS ]]; then
       
        processId="$(ps -u $uname -o rss,pid | sort -nr -k1 | head -n 1 | awk '{print $2}')"
        kill -SIGKILL "$processId" 2>/dev/null
        
        # to test
        #echo -e "kill -SIGKILL \"$processId\""
    fi
done < <(echo "$data" | tail -n +2)
