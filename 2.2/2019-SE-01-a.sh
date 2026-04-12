#!/bin/bash

# not sure if understood the problem right?

max='-1'
while read line; do
    [[ $line =~ ^-?{0,1}[0-9]+$ ]] || continue
    
    (( abs = line < 0 ? -line : line ))
    
    if (( abs >= max )); then
        if (( line < 0 )); then 
            hasNegative='1'
        else
            hasNegative='-1'
        fi

        max="$abs"
    fi
done

[[ $max -eq -1 ]] && exit 0

echo "$max"
[[ $hasNegative -eq 1 ]] && echo "-$max"

