files = system("ls -f $0")
numFiles = system("ls $0 | wc -l")
columnIndex = "$1"

print "[INFO] [plot_multicastTime.gp] first arg: $0; second arg: $1"

# print "[INFO] [plot.gp] files:".files

#set logscale y 10

set for [i = 1:numFiles+1] linetype i linewidth 2 pointsize 1.25

set key top left
set xlabel "$$n$$ parallel degree"
set ylabel "multicast service time $$(\\,\\tau\\,)$$"
plot [1:60][0:2100] for [i in files] i using 1:2 with linespoints title column(columnIndex)
