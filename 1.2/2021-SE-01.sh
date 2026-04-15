#!/bin/bash

[[ -n $ORACLE_HOME ]] || { echo "ORACLE_HOME env not defined!"; exit 1; }

[[ -d $ORACLE_HOME ]] || { echo "$ORACLE_HOME must be a directory!"; exit 2; }

# kak po dqvolite da testvam vsichko tva
# fuck this shit

