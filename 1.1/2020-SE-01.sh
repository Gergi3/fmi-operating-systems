#!/bin/bash

find ~ -type f -perm 655 | xargs --no-run-if-empty chmod g+w
