#!/bin/bash

toSearchDir=$HOME

find $toSearchDir -type f | xargs stat --printf='%Y %h %i\n' | sort -nr | head -n 1 | awk '$2 >= 2 { print $3 }'
