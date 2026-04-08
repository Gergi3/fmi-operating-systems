#!/bin/bash

filename="$HOME/oldpasswd"

awk -F: '$0 ~ /.*Inf.*/ && ($5 ~ /^[^,]*a,/ || $5 ~ /^.*a$/)' $filename | cut -c3,4 | sort | uniq -c | sort -rn | head -n 1
