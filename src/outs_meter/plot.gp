file = "$0"

set style data histograms
set style histogram cluster
#set style fill solid 1.0 border lt -1
set style fill pattern border lt -1
set boxwidth 0.9 relative

plot for [COL=2:5] file using COL:xticlabels(1) title column(COL)