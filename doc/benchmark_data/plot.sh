#!/bin/bash

if [ $# -lt 11 ] ; then 
    echo "Error: $*
Error: i parametri ($#) non sono inseriti in modo corretto: devono essere 11, i primi nove specificano l'insieme dei file di ingresso, il decimo specifica il nome delprefisso dei files di uscita, l'undicesimo specifica che script di plot eseguire:
1){gather, nogather}
2){Int, Float}
3)implementazione <int,int>
4)stram length
5)interarrival time (in clock cicles)
6)data size (vector size)
7)measure {\"\", _calctime.dbg, _calctimerow.dbg, _asymtime.dbg, _servicetime.dat, _multictime.dbg}
8)indice della colonna che fornisce il titolo dei grafici nel plot
9)titolo del grafico
10)nome del prefisso dei files di uscita
11)todo: {1 <=> scalabilit\`a, 2 <=> row calc. time}"
    exit 1
fi

MODE_TESI=0
MODE_SLIDE=1
MODE_STR=("" "_slide_2")
MODE=${MODE_SLIDE}
# plot_scalability.gp plot_T-log_millis_2.gp plot_multicastTime-millis.gp
# T-UDN-SM            comp-T-s               Tmult-nb-UDN-SM

GATHER="$1"
TYPE="$2"
IMPL="$3"
STREAMLENGTH="$4"
TA="$5"
DATASIZE="$6"
if [ "" = "$7" ] ; then MEASURE=".dat" ; else MEASURE="$7" ; fi
GRAPHTITLE_COLUMNINDEX="$8"
PLOT_TITLE="$9"

#ls ${GATHER}${TYPE}_${IMPL}_${STREAMLENGTH}_${TA}_${DATASIZE}${MEASURE}.avg

INPUT_FILE="${GATHER}${TYPE}_${IMPL}_${STREAMLENGTH}_${TA}_${DATASIZE}${MEASURE}.avg"

OUTPUT_FILE_PREFIX="${10}${MODE_STR[${MODE}]}"


TODO="${11}"

if [ $# -eq 12 ] ; then 
    PRODUCE_MAIN_TEX="true"
fi

case ${TODO} in 
    (1 | scalability) 
	SCRIPT_NAME="plot_scalability.gp"
	;;
    (2 | rowCalcTime)
	SCRIPT_NAME="plot_rowCalcTimeRatio.gp"
	;;
    (3 | complTime_log)
	SCRIPT_NAME="plot_Tc-log.gp"
	;;
    (3 | complTime_log-millis)
	SCRIPT_NAME="plot_Tc-log-millis.gp"
	;;
    (4 | complTime)
	SCRIPT_NAME="plot_Tc.gp"
	;;	
    (5 | serviceTime_log)
	SCRIPT_NAME="plot_T-log.gp"
	;;
    (serviceTime-millis_log)
	SCRIPT_NAME="plot_T-log_millis.gp"
	;;
    (serviceTime-millis_log_withIdeal*)
	T_CALC=${TODO#serviceTime-millis_log_withIdeal}
	PLOTSCRIPT_OTHER_PARAM="${T_CALC}"
	SCRIPT_NAME="plot_T-log_millis_2.gp"
	;;
    (prova)
	SCRIPT_NAME=prova.gp
	;;
    (6 | serviceTime)
	SCRIPT_NAME="plot_T.gp"
	;;	
    (7 | multicastTime)
	SCRIPT_NAME="plot_multicastTime.gp"
	;;	
    (multicastTime-millis)
	SCRIPT_NAME="plot_multicastTime-millis.gp"
	;;
esac

#echo "[INFO] [plot.sh] ${INPUT_FILE}"
#ls "${INPUT_FILE}"

/usr/local/bin/gnuplot -e "set term epslatex color solid ; 
set output \"${OUTPUT_FILE_PREFIX}.tex\" ; 
set linetype 1 pt 2 ;
call \"${SCRIPT_NAME}\" \"${INPUT_FILE}\" ${GRAPHTITLE_COLUMNINDEX} ${PLOTSCRIPT_OTHER_PARAM}" &&
epspdf "${OUTPUT_FILE_PREFIX}.eps" 

#set title \"${PLOT_TITLE}\" ; 

if [ ${PRODUCE_MAIN_TEX} ] ; then 
    if [ ! -f "${OUTPUT_FILE_PREFIX}_main.tex" ] ; then
	echo "\documentclass[a4paper]{article}
\usepackage{fullpage, caption, float, color, placeins, latexsym, amssymb, graphicx}
\begin{document}
\begin{figure}[ht] \resizebox{\columnwidth}{!}{ \input{${OUTPUT_FILE_PREFIX}.tex} } \end{figure}
\end{document}" > "${OUTPUT_FILE_PREFIX}_main.tex"
    fi
    pdflatex "${OUTPUT_FILE_PREFIX}_main.tex" &&
    open "${OUTPUT_FILE_PREFIX}_main.pdf" 
fi
