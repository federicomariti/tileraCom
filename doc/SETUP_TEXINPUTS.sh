#!/bin/bash
export TEXINPUTS=
TEXINPUTS=".:frontespizio:figures:benchmark_data:meter_data:"`ls -1 /usr/local/texlive/2009/texmf-dist/tex/latex  | tr "\n" ":"`
