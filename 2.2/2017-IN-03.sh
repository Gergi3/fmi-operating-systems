#!/bin/bash

while read line; do
    find "$line" -type f -print0 2>/dev/null \
        | xargs -r -0 stat --printf '%Y %n\n' 2>/dev/null \
        | sort -nr -k1 \
        | head -n 1 
done < <(cat /etc/passwd | cut -d: -f6)

