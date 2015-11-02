#!/bin/sh

# Dieses Programm prüft (sehr simplifiziert)
# die Richtlinien für OSUE

progname=$0
file=$1
dir=$(dirname "$file")
start_dir=$(pwd)

usage(){
	echo "usage: $progname file.c"
	exit 1
}
grp(){
	egrep "$*" "$file" > /dev/null
	return $?
}
grpfunc(){
	grp "[^A-Za-z0-9_]$*[ ]*\("
	return $?
}
errors_count=0
error(){
	printf "$*\n"
	errors_count=$((errors_count+1))
}


[ ! -f "$file" ] && usage

# Richtlinie 1

GCC_LINE="gcc -std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g -c $file"

$GCC_LINE 2>&1 > /dev/null
if [ $? -ne 0 ]; then
	error "#1: your program cannot be compiled with\n$GCC_LINE"
fi

# Richtlinie 2

cd $dir

make all 2>&1 >/dev/null
if [ $? -ne 0 ]; then
	error "#2: target make all fails"
fi
make clean 2>&1 >/dev/null
if [ $? -ne 0 ]; then
	error "#2: target make clean fails"
fi

cd $start_dir

# Richtlinie 3 - kein Test

# Richtlinie 4

check_funcs="gets scanf fscanf atoi atol"

for f in $check_funcs
do
	if grpfunc "$f"; then
		error "#4: use of $f is not allowed!"
	fi
done

# Richtlinie 5

mainfunc=0
if grp '^int[ ]+main[ ]*\(.*\)$'; then
	mainfunc=1
fi

if [ $mainfunc -ne 0 ]; then
	if ! grp "getopt"; then
		error "#5: possibly missing argument parsing (getopt)"
	fi
fi

# Richtlinie 6 - kein Test

# Richtlinie 7 - kein Test

# Richtlinie 8 - kein Test

#func=""
#while read -r line
#do	
#	if [ -z "$func" ]; then
#		echo $line | egrep '^[a-zA-Z_][a-zA-Z0-9_]*([ ]+[a-zA-Z_][a-zA-Z0-9_]*)+[ ]*\(.*\)[^;]*$' > /dev/null
#		if [ $? -eq 0 ]; then
#			func=$(echo $line | sed -r 's/([a-zA-Z_][a-zA-Z0-9_]*)[ ]*\(/\1/')
#			echo "func : $func"
#			echo "line : $line"
#		fi
#
#	fi
#done < "$file"

# Richtlinie 9 

GREPLINE="^[a-zA-Z_][a-zA-Z0-9_]*([ ]+[a-zA-Z_][a-zA-Z0-9_]*)+[ ]*\([ ]*\)[^;]*$"
if grp "$GREPLINE" ; then
	LINE=$(egrep "$GREPLINE" "$file")
	error "#9: possibly missing (void) in parameter list\n >> $LINE"
fi


# Richtlinie 10 - kein Test

# Richtlinie 11

# TODO


# Report

if [ $errors_count -ne 0 ]; then
	error "found $errors_count errors!"
	error "please check https://ti.tuwien.ac.at/cps/teaching/courses/osue/richtlinien"
	exit 1
else
	echo "your program seems to be okay ;)"
fi


