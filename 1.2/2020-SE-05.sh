#!/bin/bash

[[ $# -eq 3 ]] || { echo "3 args required" >&2; exit 1; }

[[ -f $1 ]] || { echo "$1 must be a file" >&2; exit 2; }

[[ ! -e $2 ]] || { echo "$2 mustn't exist" >&2; exit 4; }

[[ -d $3 ]] || { echo "$3 must be a directory" >&2; exit 3; }

pswd="$1"
config="$2"
cfgdir="$(realpath $3)"

while read -r file; do

    err_output="$(cat "$file" \
        | awk '{cnt++} {print "Line " cnt ":" $0 }' \
        | grep -vE '^Line [0-9]+:(\{ [a-zA-Z;-]+ \};$|#.*$|$)' \
        | awk -v file="$file" 'NR == 1 { print "Error in", file } { print $0 }')"

    if [[ -n $err_output ]]; then
        echo "$err_output"
        continue
    fi
    
    cat "$file" >> "$config"
    
    username="$(basename "$file" | sed -E 's/(.*).cfg/\1/')"

    if ! grep -qE "^$username:.*\$" "$pswd"; then
        password="$(pwgen 16 1 | sha256sum | awk '{print $1}' | cut -c1-32)"
        echo "$username:$password" >> "$pswd"
        echo "$username:$password"
    fi

done < <(find "$cfgdir" -regextype posix-extended -regex '.*\.cfg')

