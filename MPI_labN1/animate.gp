iterator=iterator+1
offset=iterator/(iterations+1.0) #1.0 нужно, чтобы все выражение вычислялось как float
if (offset <= 1.0)\
   set output "figures/".iterator.".png";\
   plot "./output.txt" index iterator using 2:3 with lines;\
   reread 