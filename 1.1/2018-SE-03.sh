#!/bin/bash

cat /etc/passwd | sort -t: -k1 | awk -F[':,'] '$9 == "SI" {print $5 ":" $10}'
