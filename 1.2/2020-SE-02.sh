#!/bin/bash

[[ $# -eq 1 ]] || { echo "One arg required"; exit 1; }

[[ -f $1 ]] || { echo "$1 must be a file"; exit 2; }

expected="$(cat "$1" | wc -l)"
actual="$(cat "$1" \
    | grep -E '^[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3} .+ .+ \[[0-9]{2}/[a-zA-Z]+/[0-9]{1,4}:[0-9]{2}:[0-9]{2}:[0-9]{2} [+-][0-9]{4}\] (GET|POST) /.* HTTP/[0-9]\.[0-9] [0-9]+ [0-9]+ ".*" ".*"$' \
    | wc -l)"

[[ $expected -eq $actual ]] || { echo "File has invalid formatting"; exit 3; }

biggest="$(cat "$1" \
    | cut -d' ' -f2 \
    | sort \
    | uniq -c \
    | sort -nr \
    | head -n 3 \
    | awk '{print $2}')"

while read -r hostname; do
    data="$(cat "$1" \
        | grep -E "^[0-9.]+ $hostname .*$")"
    
    all="$(echo -e "$data" \
        | wc -l)"

    http2="$(echo -e "$data" \
        | grep -E "^[0-9.]+ $hostname .* HTTP/2.0 .*$" \
        | wc -l)"

    (( nonHttp2 = all - http2 ))
    echo "$hostname HTTP/2.0: $http2 non-HTTP/2.0: $nonHttp2"

    echo "$data" \
        | awk '$9 > 302 {print $1}' \
        | sort \
        | uniq -c \
        | sort -nr \
        | head -n 5

done < <(echo "$biggest")
