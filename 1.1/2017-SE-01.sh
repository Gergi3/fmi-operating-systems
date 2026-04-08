#!/bin/bash

find ${PWD} -type f -maxdepth 1 -printf "%n %P\n" 2>/dev/null | sort -nr | head -n 5 | awk '{print $2}'
