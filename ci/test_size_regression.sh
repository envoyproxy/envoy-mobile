#!/bin/bash

# Checks the absolute size, and the relative increase of a file.
SIZE1=$(stat -f "%z" "$1")
SIZE2=$(stat -f "%z" "$2")
PERC=$(bc <<< "scale=2; ($SIZE2 - $SIZE1)/$SIZE1 * 100")
echo "The new binary is $PERC % different in size compared to master"

echo "The new binary is $SIZE2 bytes"
