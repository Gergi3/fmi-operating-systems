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

info="$(ps -e -o uid,pid,rss,uname | tail -n +2 | sort -k1 | awk '
    BEGIN {
        {print "UNAME UID SUM_RSS MAX_PID" }
        max_pid = -1
        last_uid = -1
        last_uname = -1
        sum_rss = 0
        first = 1
    }
    {
        if (last_uid != $1) {
            if (first == 1) {
                first = 0;
            } else {
                { print last_uname, last_uid, sum_rss, max_pid }
            }

            max_pid = $2
            sum_rss = $3
        } else {
            if ($3 > max_pid_rss) {
                max_pid = $2
                max_pid_rss = $3
            }
            sum_rss = sum_rss + $3
        }
        last_uid = $1
        last_uname = $4
    }
    END {
        print last_uname, last_uid, sum_rss, max_pid
    }
' | column -t)"


echo "$info"

while read -r uname uid rssSum maxPid; do
    if [[ $rssSum -gt $MAX_RSS ]]; then
        # to test
        #echo -e "kill -SIGKILL \"$maxPid\""
        kill -SIGKILL "$maxPid"
    fi
done < <(echo "$info" | tail -n +2)
