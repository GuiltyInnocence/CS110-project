#!/usr/bin/env bash

outFile=test.out
infile=test.in

while true
do
    ./a.out
    ./translator "$infile" ${outFile} > /dev/null
    ./template "$infile" "temp.out" > /dev/null
    diff ${outFile} "temp.out"
    read
done