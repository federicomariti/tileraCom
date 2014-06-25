files = system("ls -f $0")
numFiles = system("ls $0 | wc -l")
columnIndex = "$1"
Tcalc = "$2"

print "[INFO] [plot_T-log_millis_2.gp] first arg: $0; second arg: $1 other: $2"

# print "[INFO] [plot.gp] files:".files

set logscale y 10

set for [i = 2:numFiles+1] linetype i linewidth 2 pointsize 1.25
call "gnuplot_tesi.gp" 1
call "gnuplot_slide.gp" 0

set key top left
set xlabel "$$n$$ parallel degree"
set ylabel "service time ($$\\,\\mu\\mathrm{sec}$$\\,)"
#plot [1:60] Tcalc/x title "ideal service time", for [i in files] i using 1:9 with linespoints title column(columnIndex)
plot [1:60] for [i in files] i using 1:9 with linespoints title column(columnIndex)