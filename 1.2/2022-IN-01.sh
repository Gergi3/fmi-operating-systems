#!/bin/bash

[[ $# -eq 2 ]] || { echo "Wrong number of args, expected: <dir1> <empty_dir2>" >&2; exit 1; }

[[ -d $1 ]] || { echo "$1 must be a directory" >&2; exit 2; }

[[ -d $2 ]] || { echo "$2 must be an empty directory" >&2; exit 3; }

[[ $(find "$2" -mindepth 1 | wc -l) -eq 0 ]] || { echo "$2 must be an empty directory" >&2; exit 4; }

# to avoid ambiguity
src="$(realpath $1)"
dst="$(realpath $2)"

cp -r $src/* "$dst"
all="$(find "$dst" -type f)"

while read -r file; do
    dir_name="$(dirname "$file")"
    file_name="$(basename "$file")"
    if echo "$all" | grep -E "^$dir_name/.$file_name.swp$"; then
        rm -f "$dir_name/.$file_name.swp"
    fi
done < <(echo "$all")

