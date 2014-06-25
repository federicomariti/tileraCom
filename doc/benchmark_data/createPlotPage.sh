#!/bin/bash

if [ $# -ne 1 ] ; then 
    echo "Error: richiesto un unico parametro, il prefisso del plot"
    exit 1
fi

echo "\documentclass[a4paper]{article}
\usepackage{fullpage, caption, float, color, placeins, latexsym, amssymb, graphicx}
\begin{document}
\begin{figure}[ht] \resizebox{\columnwidth}{!}{ \input{${1}.tex} } \end{figure}
\end{document}" > "${1}_main.tex"

pdflatex "${1}_main.tex" && open "${1}_main.pdf" 
