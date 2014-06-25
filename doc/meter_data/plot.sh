#!/bin/bash

SHOW_PLOT="true"

if [ $# -ne 2 ] ; then 
    echo "Error: inserire il parametro Tipo e il parametro Numero di Iterazioni (il logaritmo base 10)"
    exit 1
fi

TYPE="$1"
NITER="$2"
INPUT_FILE="${TYPE}_${NITER}_sym_all.dat.avg"
OUTPUT_FILE_PREFIX="${TYPE}_${NITER}_sym_all"
FILL_SOLID=1
FILL_PATTERN=2
TODO_SYM=1
COLOR=2

if [ ! -f "${OUTPUT_FILE_PREFIX}_main.tex" ] ; then
    echo "\documentclass[a4paper]{article}
\usepackage{fullpage, caption, float, color, placeins, latexsym, amssymb, graphicx}
\begin{document}
\begin{figure}[ht] \resizebox{\columnwidth}{!}{ \input{${OUTPUT_FILE_PREFIX}.tex} } \end{figure}
\end{document}" > "${OUTPUT_FILE_PREFIX}_color_main.tex"
fi

/usr/local/bin/gnuplot -e "set term epslatex color solid ; set output \"${OUTPUT_FILE_PREFIX}_color.tex\" ; call \"plot.gp\" \"${INPUT_FILE}\" \"1\" \"${FILL_SOLID}\" \"${COLOR}\"" && epspdf "${OUTPUT_FILE_PREFIX}_color.eps" 

/usr/local/bin/gnuplot -e "set term epslatex color solid ; set output \"${OUTPUT_FILE_PREFIX}_bw.tex\" ; call \"plot.gp\" \"${INPUT_FILE}\" \"1\" \"${FILL_PATTERN}\" \"${COLOR}\"" && epspdf "${OUTPUT_FILE_PREFIX}_bw.eps" 

if [ "${SHOW_PLOT}" ] ; then
    pdflatex "${OUTPUT_FILE_PREFIX}_color_main.tex" && open "${OUTPUT_FILE_PREFIX}_color_main.pdf"
fi



INPUT_FILE="${TYPE}_${NITER}_asymin_all.dat.avg"
OUTPUT_FILE_PREFIX="${TYPE}_${NITER}_asymin_all"

if [ ! -f "${OUTPUT_FILE_PREFIX}_color_main.tex" ] ; then
    echo "\documentclass[a4paper]{article}
\usepackage{fullpage, caption, float, color, placeins, latexsym, amssymb, graphicx}
\begin{document}
\begin{figure}[ht] \resizebox{\columnwidth}{!}{ \input{${OUTPUT_FILE_PREFIX}_color.tex} } \end{figure}
\end{document}" > "${OUTPUT_FILE_PREFIX}_color_main.tex"
fi

/usr/local/bin/gnuplot -e "set term epslatex color solid ; set output \"${OUTPUT_FILE_PREFIX}_color.tex\" ; call \"plot.gp\" \"${INPUT_FILE}\" 2 \"${FILL_SOLID}\" \"${COLOR}\"" && epspdf "${OUTPUT_FILE_PREFIX}_color.eps"

/usr/local/bin/gnuplot -e "set term epslatex color solid ; set output \"${OUTPUT_FILE_PREFIX}_bw.tex\" ; call \"plot.gp\" \"${INPUT_FILE}\" 2 \"${FILL_PATTERN}\" \"${COLOR}\"" && epspdf "${OUTPUT_FILE_PREFIX}_bw.eps"

if [ "${SHOW_PLOT}" ] ; then
    pdflatex "${OUTPUT_FILE_PREFIX}_color_main.tex" && open "${OUTPUT_FILE_PREFIX}_color_main.pdf"
fi
