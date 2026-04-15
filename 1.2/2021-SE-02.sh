#!/bin/bash

fdqn_regex='[a-z0-9]{1}(\.?[a-z0-9])*\.'
echo $@
for file in "$@"; do
    [[ -f $file ]] || { echo "$file not found"; continue; }
    data="$(cat "$file" | tr -s '[:blank:]' | awk -F';' '{print $1}' | awk '$1=$1 {print $0}')"
    
    # validate soa record
    if echo "$data" | head -n 7 | grep -qPz "^$fdqn_regex ([0-9]+ )?IN SOA $fdqn_regex $fdqn_regex \(\n[0-9]{10}\n[0-9]+\n[0-9]+?\n[0-9]+\n[0-9]+\n\)(\n.*)?"; then
        soa_lines=8
    elif echo "$data" | head -n 1 | grep -qE "^$fdqn_regex ([0-9]+ )?IN SOA $fdqn_regex $fdqn_regex [0-9]{10} [0-9]+ [0-9]+? [0-9]+ [0-9]+$"; then
        soa_lines=2
    else
        echo "$file - Invalid first row, record type must be a valid singleline/multiline SOA" >&2
        continue
    fi
   
    # validate other records
    if echo "$data" | tail -n +$soa_lines | grep -qvE "^$fdqn_regex ([0-9]+ )?IN (NS|A|AAAA) $fdqn_regex $fdqn_regex [0-9]{10} [0-9]+ [0-9]+ [0-9]+ [0-9]+$|^$"; then
        echo "$file - Invalid data format" >&2
        continue
    fi

    # change date
    curr_serial="$(grep -oE '[0-9]{8}[0-9]{2}' $file)"
    serial_date="$(echo "$curr_serial" | cut -c1-8)"
    curr_date="$(date +'%Y%m%d')"

    if [[ $serial_date < $curr_date ]]; then
        new_serial="${curr_date}00"
    elif [[ $serial_date == $curr_date ]]; then
        (( new_serial = curr_serial + 1 ))
    fi

    sed -i -E "s/(.*)([0-9]{10})(.*)/\1$new_serial\3/" "$file"
done

