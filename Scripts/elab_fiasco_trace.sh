#!/bin/bash

tr -cd '\11\12\40-\176' < $1 > file.tmp
grep SCHED file.tmp | cut -d ' ' -f 3,5,10- > res.tmp
cat res.tmp | grep -v clear  > res2.tmp
tr '[' ' ' < res2.tmp | tr '+' ' ' |awk '{if ($4=="ms") print $1,$2,$3*1000; else if ($4=="s") print $1,$2,$3*1000000; else print $1,$2,substr($3,1,3)}' > res3.tmp
tac res3.tmp > res4.tmp
head -n -$2 res4.tmp
rm *.tmp 
