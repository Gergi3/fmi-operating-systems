#!/bin/bash

# comment out to test
[[ $(whoami) == "root" ]] || { echo "You must be logged in as root"; exit 1; }

[[ $# -eq 3 ]] || { echo "Invalid number of args"; exit 2; }
[[ -d $1 ]] || { echo "$1 must be an existing directory"; exit 3; }
[[ -d $2 ]] || { echo "$2 must be an existing directory"; exit 4; }
[[ $1 != $2 ]] || { echo "$1 and $2 must be different"; exit 5; }
[[ "$(find $2 -mindepth 1 | wc -l)" -eq 0 ]] || { echo "$2 must be an empty directory"; exit 6; }

src="$(realpath $1)" # realpath to avoid ambiguity such as difference when passed ~/src and ./src and /home/students/s0600421/src
dest="$(realpath $2)"
toMatch="$3"

toMoveAll="$(find "$src" -regextype posix-extended -regex ".*$toMatch.*" -printf "%P\n")"

while read toMove; do
    from="$src/$toMove"
    to="$dest/$toMove"

    mkdir -p "$(dirname "$to")"
    cp --no-preserve=all "$from" "$to"
    rm -f "$from"
done < <(echo "$toMoveAll")
