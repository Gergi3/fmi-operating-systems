#!/bin/bash

[[ $# -eq 2 ]] || { echo "Two args required"; exit 1; }

[[ -d $1 ]] || { echo "$1 must be an existing directory"; exit 2; }

[[ ! -e $2 ]] || { echo "$2 mustn't be an existing file or directory"; exit 3; }

src="$(realpath $1)"
dst="$2"

mkdir -p "$dst/images"

while read -r filename; do
    
    title="$(basename "$filename" \
        | sed -E 's/\(.*\)/ /g' \
        | sed -E 's/(.*)\.jpg/\1/g' \
        | awk '$1=$1 {print $0}')"

    album="$(basename "$filename" \
        | sed -E '/^.*\(.*\).*$/!s/^.*$/misc/' \
        | sed -E 's/^.*(.*\(.*\).*)*\((.*)\).*$/\2/' \
        | awk '$1=$1 {print $0}')"

    date="$(stat "$filename" --printf '%y' | cut -d' ' -f1)"
    
    checksum="$(sha256sum "$filename" | cut -c1-16)"
    
    mkdir -p "$dst/by-date/$date/by-album/$album/by-title" 2>/dev/null
    mkdir -p "$dst/by-date/$date/by-title" 2>/dev/null
    mkdir -p "$dst/by-album/$album/by-date/$date/by-title" 2>/dev/null
    mkdir -p "$dst/by-album/$album/by-title" 2>/dev/null
    mkdir -p "$dst/by-title" 2>/dev/null

    cp "$filename" "$dst/images/$checksum.jpg"
    absolute_path=$(realpath "$dst/images/$checksum.jpg")
    ln -s "$absolute_path" "$dst/by-date/$date/by-album/$album/by-title/$title.jpg"
    ln -s "$absolute_path" "$dst/by-date/$date/by-title/$title.jpg"
    ln -s "$absolute_path" "$dst/by-album/$album/by-date/$date/by-title/$title.jpg"
    ln -s "$absolute_path" "$dst/by-album/$album/by-title/$title.jpg"
    ln -s "$absolute_path" "$dst/by-title/$title.jpg"

done < <(find "$src" -regextype posix-extended -regex '^.*\.jpg$')

