#!/bin/bash

[[ $# -eq 3 ]] || { echo "3 args required: <configuration_file> <key> <value>" >&2; exit 2; }

[[ -f $1 ]] || { echo "$1 must be an existing configuration file" >&2; exit 1; }

function get_regex() {
    echo "^[[:blank:]]*$1[[:blank:]]*\=[[:blank:]]*$2[[:blank:]]*(#.*)?$"
}

key="$2"
value="$3"

if grep -qvE '^[[:blank:]]*([a-zA-Z0-9_]+[[:blank:]]*\=[[:blank:]]*[a-zA-Z0-9_]+[[:blank:]]*)?(#.*)?$' "$1"; then
    echo "Invalid configuration file format!" >&2
    exit 3
elif grep -qE "$(get_regex $key $value)" "$1"; then
    exit 0;
elif grep -qE "$(get_regex $key '[a-zA-Z0-9_]+')" "$1"; then
    sed -i -E "s/$(get_regex $key '[a-zA-Z0-9_]+')/#\0 # edited at $(date) by $(whoami)\n$key \= $value # added at $(date) by $(whoami)/" "$1"
else
    echo "$value # added at $(date) by $(whoami)" >> $1
fi

