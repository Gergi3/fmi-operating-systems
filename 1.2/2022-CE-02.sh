#!/bin/bash

[[ $# -eq 1 ]] || { echo "One arg required"; exit 1; }

[[ $1 =~ ^[0-9A-Z]{1,4}$ ]] || { echo "$1 must be max 4 chars long, all uppercase latin letters or numbers"; exit 2; }

wakeup_file="/proc/acpi/wakeup"
data_normalized="$(cat "$wakeup_file" | tail -n +2 | awk -F'[\t ]+' '{print $0}')"

if ! echo "$data_normalized" | grep -qE "^$1 .*$"; then
    echo "Device not in $wakeup_file" >&2
    exit 3
fi

if echo "$data_normalized" | grep -qE "^$1 .* \*enabled .*$"; then
    echo "$1" > "$wakeup_file"
fi

