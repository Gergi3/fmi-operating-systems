#!/bin/bash

echo -e "A)"
find $PWD -type f -size 0c -exec echo 'deleted' {} \; 
 
echo -e "\nB)"
find ~ -type f -print0 | xargs --null wc -c --total=never | sort -rn | head -n 5 | awk '{print $2}'
