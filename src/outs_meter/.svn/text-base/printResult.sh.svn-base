#!/bin/bash

if [ $# -lt 2 ] ; then
    echo "Error: necessari parametri Tipo e Numero Scambi, opzionale una terza stringa qualsiasi per calcolare le medie"
    exit 1
fi

TYPE="$1"
NSCAMB="$2"
CALCULATE_AVG="$3"

if [ -n "$CALCULATE_AVG" ] ; then
    ./compact ${TYPE}_${NSCAMB}_*.dat
    echo asdf
fi

./printAll ${TYPE} ${NSCAMB}

./plot.sh ${TYPE} ${NSCAMB}

echo UDN 
tail -n3 ${TYPE}_${NSCAMB}_sym_all_stat.dat.avg | cut -d " " -f 1,2-4 | sed "s/ / \& /g"

echo SM_NO
tail -n3 ${TYPE}_${NSCAMB}_sym_all_stat.dat.avg | cut -d " " -f 1,5-7 | sed "s/ / \& /g"

echo SM_FENCE
tail -n3 ${TYPE}_${NSCAMB}_sym_all_stat.dat.avg | cut -d " " -f 1,8-10 | sed "s/ / \& /g"

echo SM_NULLACK
tail -n3 ${TYPE}_${NSCAMB}_sym_all_stat.dat.avg | cut -d " " -f 1,11-13 | sed "s/ / \& /g"


echo

echo UDN 
tail -n3 ${TYPE}_${NSCAMB}_asymin_all_stat.dat.avg | cut -d " " -f 1,2-4 | sed "s/ / \& /g"

echo SM_NO
tail -n3 ${TYPE}_${NSCAMB}_asymin_all_stat.dat.avg | cut -d " " -f 1,5-7 | sed "s/ / \& /g"

echo SM_FENCE
tail -n3 ${TYPE}_${NSCAMB}_asymin_all_stat.dat.avg | cut -d " " -f 1,8-10 | sed "s/ / \& /g"

