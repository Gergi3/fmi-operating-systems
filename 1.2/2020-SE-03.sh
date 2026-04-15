#!/bin/bash

# repo db => <pckg name>-<pckg version> <pckg checksum>
# repo packages => *.tar.xz
# package vers => 1.2.3-4
# package tree => to tar xz

[[ $# -eq 2 ]] || { echo "Two args required"; exit 9; }

package=$2
[[ -d $package ]] || { echo "$package must be an existing directory"; exit 1; }

repo=$1
[[ -d $repo ]] || { echo "$repo must be an existing directory"; exit 2; }

package="$(realpath "$package")"
[[ "$(find "$package" -maxdepth 1 -mindepth 1 | wc -l)" -eq 2 ]] || { echo "$package must be a valid package dir"; exit 3; }

repo="$(realpath "$repo")"
[[ "$(find "$repo" -maxdepth 1 -mindepth 1 | wc -l)" -eq 2 ]] || { echo "$repo must be a valid repository dir"; exit 4; }

package_version_file="$package/version"
[[ -f $package_version_file ]] || { echo "$package must have a version file"; exit 5; }

package_tree="$package/tree"
[[ -d $package_tree ]] || { echo "$package must have a tree directory"; exit 6; }

repo_db="$repo/db"
[[ -f $repo_db ]] || { echo "$repo must have a db file"; exit 7; }

repo_packages="$repo/packages"
[[ -d $repo_packages ]] || { echo "$repo must have a packages directory"; exit 8; }

tar -c "$package_tree" -f "$package/archived.tar" 2>/dev/null
package_checksum="$(sha256sum "$package/archived.tar" | awk '{print $1}')"

xz "$package/archived.tar"
mv "$package/archived.tar.xz" "$repo_packages/$package_checksum.tar.xz"

package_name="$(basename "$package")"
package_version="$(cat "$package_version_file")"
db_key="$package_name-$package_version"

temp=$(mktemp)
cat "$repo_db" > $temp

if grep -qE "^$db_key [a-zA-Z0-9]{64}$" $temp; then
   sed -iE "s/^$db_key [a-zA-Z0-9]{64}$/$db_key $package_checksum/g" $temp
else
    echo "$db_key $package_checksum" >> $temp
fi

sort $temp > "$repo_db"

rm $temp
