#!/bin/bash

find ~ -mindepth 1 -maxdepth 1 -type f -user $(whoami) 2>/dev/null | xargs -r chmod 664
