file = "$0"
todo = "$1"
fill = "$2"
color = "$3"

FILL_SOLID = 1
FILL_PATTERN = 2
TODO_SYM = 1
TODO_ASYMIN = 2
G_RED = 1
G_GREEN = 2

print "[plot.gp][INFO] fill ".'$2'." color ".'$3'

set xlabel "number of hops"
set ylabel "$$\\mathrm{L}_{\\mathrm{com}} \\; (\\,\\tau\\,)$$"
#set key outside center top Right
set key outside tm
set style data histograms
set style histogram cluster
set boxwidth 0.9 relative

if (fill == FILL_SOLID) {
  set style fill solid 1.0 border lt -1
  a = "#3366cc"
  if (color == G_RED) {
    b = "#ff9966"
    c = "#ff3300"
    d = "#993300"
  } 
  if (color == G_GREEN) {
    b = "#99ff99"
    c = "#33cc33"
    d = "#006600"
  }
} 
if (fill == FILL_PATTERN) {
  set style fill pattern 0 border lt -1
  a = "dark-orange"
  b = "dark-red"
  c = "sea-green"
  a = "black"
  b = "black"
  c = "black"
  d = "gray70"
}

offset_label=0
offset_label=0

if (todo == TODO_SYM) {
  end_field = 5
  if (fill == FILL_SOLID) {
    offset_label = 4
  }
}
if (todo == TODO_ASYMIN) {
  end_field = 4
  if (fill == FILL_SOLID) {
    offset_label = 3
    d = b
    b = c
    c = d
  }   
}

set linetype 1 lc rgb a
set linetype 2 lc rgb b
set linetype 3 lc rgb c
set linetype 4 lc rgb d

set yrange [0:]

set term push 
set term dumb 
plot for [COL=2:end_field] file using COL:xticlabels(1) title column(COL+offset_label)

set term pop 
set ytics nomirror
set y2range [0:GPVAL_Y_MAX*0.001156626]
set y2label "$$\\mathrm{L}_{\\mathrm{com}} \\; (\\,\\mu\\mathrm{sec}\\,)$$"
set y2tics

plot for [COL=2:end_field] file using COL:xticlabels(1) title column(COL+offset_label)