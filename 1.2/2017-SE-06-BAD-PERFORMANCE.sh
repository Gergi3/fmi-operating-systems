#!/bin/bash

is_valid_home() {
    [[ -d $1 ]] || return 1;

    local homedir_owner="$(stat --printf '%U' "$1")"
    [[ $homedir_owner == $2 ]] || return 2;

    local owner_perm="$(stat --printf '%a' "$1")"
    (( 8#$owner_perm & 0200 )) || return 3;
}

get_user_rss() {
    ps -U "$1" -o rss | tail -n +2 | awk 'BEGIN {sum=0} {sum += $1} END {print sum}' 
}

# comment to test
[[ $(whoami) == "root" ]] || { echo "You need to be logged in as root!"; exit 1; }

rootRss="$(get_user_rss "root")"
while read -r user homedir; do
    is_valid_home "$homedir" "$user" && continue
    
    currRss="$(get_user_rss "$user")"
    
    if [[ $currRss -gt $rootRss ]]; then
        ps -U "$user" -o pid | xargs kill -SIGKILL
        
        # uncomment to test
        # ps -U "$user" -o pid | tail -n +2 | xargs -I {} echo "kill -SIGKILL {}"
    fi


done < <(cut -d':' -f1,6 /etc/passwd | tr ':' ' ')
