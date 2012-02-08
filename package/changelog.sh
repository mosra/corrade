#!/bin/bash
prev=v0.2
curr=v0.2

echo -e "Corrade - changes from $(echo $prev | tr -d v) to $(echo $curr | tr -d v):\n"

git log --oneline $prev..$curr | tee
