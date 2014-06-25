offset = $0
print "[INFO] [gnuplot_tesi] offset = $0"

blue = "#3366cc"
green = "#33cc33"

set grid
if (offset == 1) {
  set linetype 1+offset lw 1 lc rgb "#ee82ee"
}
set linetype 1+offset lw 3 ps 1.25 lc rgb blue
set linetype 2+offset lw 3 ps 1.25 lc rgb green