set terminal pngcairo enhanced font "arial,10" size 800,600
set output 'inst_per_block.png'
set title "Number of Instructions Per Basic Block"
set xlabel "Instructions"
set ylabel "Frequency"
set style fill solid 0.5
set grid y
set xtics ("1-9" 1, "10-99" 2, "100-999" 3, "1000+" 4)
set xrange [0.5:4.5]
plot 'bin_counts.txt' using 1:2 with boxes title "Frequency" lc rgb "#4169E1"
