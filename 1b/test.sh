#!/bin/sh

PORT=9999
COLORS="b d g o r s v w"
COLORS2=$(echo "$COLORS" | sed 's/ //g')

COMBINATIONS=0
for a1 in $COLORS
do
	for a2 in $COLORS
	do
		for a3 in $COLORS
		do
			for a4 in $COLORS
			do
				for a5 in $COLORS
				do
					COMBINATIONS=$((COMBINATIONS+1))
				done
			done
		done
	done
done

i=0

if [ -f "game.log" ]; then
	# delete combination which was not tested yet
	tail -n 1 game.log | grep "combination" > /dev/null
	if [ $? -eq 0 ]; then
		head -n -1 game.log > temp.log
		mv temp.log game.log
	fi

	start=$(tail game.log |grep 'combination:' | egrep -o "[$COLORS2]{5}" | tail -n 1)
	printf "seeking..."
	max=$(grep 'Runden' game.log | egrep -o "[0-9]+" | sort -g | tail -n1)

else
	#start=""
	max=0
fi

for a1 in $COLORS
do
	for a2 in $COLORS
	do
		for a3 in $COLORS
		do
			for a4 in $COLORS
			do
				for a5 in $COLORS
				do
					c=$a1$a2$a3$a4$a5$a6
					i=$((i+1))

					if [ $i -eq 200 ]; then
						exit 0
					fi

					if [ ! -z "$start" ]; then
						if [ "$start" != "$c" ]; then
							continue
						else
							printf "\r"
							start=""
							continue
						fi
					fi

					printf " - testing $c ... (%d/%d) max rounds: %d\r" $i $COMBINATIONS $max

					echo "combination: $c" >> game.log
					while [ 1 ]
					do
						./server $PORT $c >>game.log &
						ps | grep "server" > /dev/null
						if [ $? -eq 0 ]; then
							break
						fi
						sleep 0.2
					done


					while [ 1 ]
					do
						./client localhost $PORT > /dev/null
						if [ $? -eq 0 ]; then
							break
						fi
						sleep 0.2
					done

					echo  >> game.log

					r=$(tail game.log | grep 'Runden' | egrep -o "[0-9]+" | sort -g | tail -n1)
					if [ ! -z "$r" ]; then
						if [ $r -gt $max ]; then
							max=$r
						fi
					fi
				done
			done
		done
	done
done
printf "\ndone!\n"

max=$(grep 'Runden' game.log | egrep -o "[0-9]+" | sort -g |tail -n1)
i=1
sumz=0
sumn=0
while [ $i -le $max ]
do
	r=$(egrep "^Runden: $i$" game.log | wc -l)
	if [ $r -ne 0 ]; then
		all_i="$all_i $i"
		all_r="$all_r $r"
		printf "${r}x \t$i Runden\n"
		sumz=$((sumz+r*i))
		sumn=$((sumn+r))
	fi
	i=$((i+1))
done

avg=$(echo "scale=4;$sumz / $sumn" | bc)
printf "Average: $sumz / $sumn = $avg\n"
