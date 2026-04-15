#!/bin/bash
# 0 - before jar
# 1 - after jar, before jarfile
# 2 - after jarfile (args)
part=0

opts=$(mktemp)
args=$(mktemp)
for arg in $@; do
    if [[ $part -eq 0 ]]; then
        if [[ $arg == '-jar' ]]; then
            part=1
        elif [[ ! $arg =~ ^-D[a-zA-Z0-9]+=[a-zA-Z0-9]+$ ]]; then
            echo -n "$arg " >> $opts
        fi
    elif [[ $part -eq 1 ]]; then
        if [[ $arg =~ ^-.* ]]; then
            echo -n "$arg " >> $opts
        else
            file_name=$arg
            part=2
        fi
    elif [[ $part -eq 2 ]]; then
        echo -n "$arg " >> $args
    fi
done

echo "java $(cat $opts)-jar $file_name $(cat $args)"

rm $opts
rm $args

