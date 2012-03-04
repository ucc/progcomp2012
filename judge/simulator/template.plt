set term png size 640,480
set output "[NAME].png"
set xtics 1
set ytics 1
set xrange [1:]
set xlabel "Games Played"
set ylabel "Score"
set title "[NAME] scores"
set key top left
plot "[NAME].scores" using ($0+1):1 with linespoints title "[NAME]"
exit
