#!/bin/bash

dataFile=/srv/sample-data/01-shell/2020-SE-02/spacex.txt

launchSite="$(cat $dataFile | awk -F'|' '$3 == "Failure" {print $2}' | sort | uniq -c | sort -nr | awk '{print $2}')" 

cat $dataFile | tail -n +2 | awk -F '|' -v site=$launchSite '$2 == site {print $0}' | sort -t '|' -k1 -nr | head -n 1 | awk -F '|' '{print $3 ":" $4}'
