#!/bin/bash

# NOTE: Not sure about description of problem. Currently checking if directory exists, if user is owner of the home directory and has write access to it

# ERROR CODES
# 1 - Invalid user

# to test
required_user=$(whoami)

#required_user="root"

[[ $(whoami) == $required_user ]] || { echo "You must be logged in as $required_user" >&2; exit 1; }

while read line; do
    home_dir="$(echo "$line" | awk -F: '{print $6}')"
    username="$(echo "$line" | awk -F: '{print $1}')"
    
    [[ -d $home_dir ]] || { echo "$username"; exit 0; }
    
    home_dir_owner="$(stat --printf '%U' $home_dir 2>/dev/null)"
    home_dir_perm="$(stat --printf '%a' $home_dir 2>/dev/null | cut -c1)"

    [[ ($home_dir_owner == $username) && ($home_dir_perm -ge 4) ]] || { echo "$username"; exit 0; }
done < <(cat /etc/passwd)
