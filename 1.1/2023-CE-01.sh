#!/bin/bash

find / -regex '.*.blend[0-9]+' -user $(whoami) 2>/dev/null
