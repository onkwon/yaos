#!/bin/bash

# check stdin
if [ -t 0 ]; then exit; fi

v=`cat /dev/stdin | sed -e 's/ //g'`
len=${#v}

i=0
while [ $i -lt $len ]
do
	echo -n ${v:$i+6:2}${v:$i+4:2}${v:$i+2:2}${v:$i:2}
	i=$[$i+8];
done

echo
