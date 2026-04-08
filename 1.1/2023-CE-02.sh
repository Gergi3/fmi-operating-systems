#!/bin/bash

dirname=/srv/sample-data/01-shell/2023-CE-02/my_logs

find $dirname -type f -regextype posix-extended -regex '.*/[a-zA-Z0-9_]+_[0-9]+\.log' | xargs grep -o 'error' | wc -l
