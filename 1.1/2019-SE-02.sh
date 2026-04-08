#!/bin/bash

starts=1775128430
ends=1775298684

#FNS="$(find /home/students -mindepth 1 -maxdepth 1 | xargs stat --printf '%Z %n\n' | sort -n -k1 | awk -v start=$start -v end=$end '$1 >= start && $1 <= end {print $2}')"

paste -d ' ' \
  <( awk -F'[,:]' '{print substr($1, 2) ":" $5 ":" }' /etc/passwd) \
  <( awk -F'[,:]' '{print $10}' /etc/passwd | xargs stat --printf '%Z\n') \
  | sort -n -k3 \
  | awk -F: -v starts=$starts -v ends=$ends '$3 >= starts && $3 <= ends { print $1 " " $2 }'
