#!/bin/bash

# check stdin
if [ -t 0 ]; then exit; fi

v=`cat /dev/stdin | sed -e 's/\(\n\| \)//g'`
len=${#v}
i=8
c=0

if [ ${v:8:2} = "00" ] && [ $((16#${v:10:2})) -gt 127 ];
then i=$[$i+2];
fi

while [ $i -lt $len ]
do
	echo -n ${v:$i:2}
	i=$[$i+2];
	c=$[$c+2];
	if [ $c = 64 ];
	then i=$[$i+4]
		if [ ${v:$i:2} == "00" ] && [ $((16#${v:$i+2:2})) -gt 127 ];
		then i=$[$i+2]
		fi
	fi
done

echo
