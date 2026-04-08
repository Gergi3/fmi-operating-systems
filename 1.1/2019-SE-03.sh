#!/bin/bash

neededInode="$(find ~ -type f | xargs stat --printf '%Y %i\n' | sort -n | head -n 1 | cut -d ' ' -f2 )"

paste -d ' ' \
    <(find ~ -type f | xargs stat --printf '%n\n' | awk -F'/' '{ print NF-1 }') \
    <(find ~ -type f | xargs stat --printf ' %i\n') \
    | awk -v needed=$neededInode 'needed == $2 {print $1}' \
    | sort -n \
    | head -n 1
