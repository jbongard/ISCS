cd Movie2 
for f in *ppm ; do nice -n 20 convert -quality 100 $f `basename $f ppm`jpg; done 
rm *.ppm 
mencoder 'mf://*.jpg' -mf fps=60 -o Movie2.avi -ovc lavc 
rm *.jpg 
