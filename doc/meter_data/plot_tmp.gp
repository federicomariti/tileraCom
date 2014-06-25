file = "$0"

L = "asdf"

#set label 1 at -2, 0.5 "$erf(x) = \\frac{2}{\\sqrt{\\pi}}\\int_0^x\\, dt e^{-t^2}$" centre
set xlabel "number of hops"
set ylabel "$$L_{com} (\\tau)$$"
set key outside center top
set style data histograms
set style histogram cluster
#set style fill solid 1.0 border lt -1
set style fill pattern border lt -1
set boxwidth 0.9 relative

set yrange [0:]

set term push 
set term dumb 
plot for [COL=2:5] file using COL:xticlabels(1) title column(COL)

set term pop 
set ytics nomirror
set y2range [0:GPVAL_Y_MAX*0.001156626]
set ylabel 'bla'
set y2tics

plot for [COL=2:5] file using COL:xticlabels(1) title column(COL)