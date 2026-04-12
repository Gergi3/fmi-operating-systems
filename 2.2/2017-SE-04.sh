#!/bin/bash

[[ $# -le 2 ]] || { echo "At least 1 arg is required"; exit 1; }

[[ -d $1 ]] || { echo "$1 must be an existing directory"; exit 2; }

info="$(find $1 \( -type l ! -xtype l \) -printf '%p -> %l\n')"
brokenLinks="$(find $1 -xtype l | wc -l)"


if [[ $# -eq 1 ]]; then
 echo -e "$info\nBroken symlinks: $brokenLinks"
elif [[ $# -eq 2 ]]; then
 echo -e "$info\nBroken symlinks: $brokenLinks" > $2
fi
