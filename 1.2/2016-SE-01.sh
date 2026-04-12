#!/bin/bash

NEEDED_ARGS=1;

# ERROR CODES
# 1 - wrong number of args
# 2 - not directory/does not exist

[[ $# -eq $NEEDED_ARGS ]] || { echo "Invalid number of args (expected: $NEEDED_ARGS, got: $#)" >&2; exit 1; }

DIRNAME=$1

[[ -d $DIRNAME ]] || { echo "$DIRNAME does not exist or is not a directory" >&2; exit 2; }

while read -r symlink; do
    # realink -qe "$symlink" can alternatively be used to follow chained symlinks
    if [[ -e $(readlink -q "$symlink") ]]; then
        echo -n "[VALID]"
    else
        echo -n "[INVALID]"
    fi

    echo "$symlink"
done < <(find $DIRNAME -type l)
