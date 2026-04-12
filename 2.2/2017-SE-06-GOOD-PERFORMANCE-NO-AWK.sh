#!/bin/bash

is_valid_home() {
    [[ -d $1 ]] || return 1;

    local homedir_owner="$(stat --printf '%U' "$1")"
    [[ $homedir_owner == $2 ]] || return 2;

    local owner_perm="$(stat --printf '%a' "$1")"
    (( 8#$owner_perm & 0200 )) || return 3;
}

# comment to test
[[ $(whoami) == "root" ]] || { echo "You need to be logged in as root!"; exit 1; }

declare -A usersRss
declare -A userPids
while read -r user pid rss; do
    (( usersRss[$user] += rss ))
    userPids[$user]+="$pid "
done < <(ps -e -o uname,pid,rss | tail -n +2)

rootRss="${usersRss["root"]}"
while read -r user homedir; do
    if is_valid_home "$homedir" "$user"; then
        continue
    fi
    
    if [[ ${usersRss[$user]} -gt $rootRss ]]; then
        kill -SIGKILL ${userPids[$user]}
        
        # uncomment to test
        #echo "kill -SIGKILL ${userPids[$user]}"
    fi


done < <(cut -d':' -f1,6 /etc/passwd | tr ':' ' ')

