default:
	gcc -o kostex kostex.c -I/usr/include/cairo -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/pixman-1 -lcairo -lm -g
	./kostex
	ls *.svg | xargs -I ^ -P 4 ./process.sh ^

test:
	gcc -o kostex kostex.c -I/usr/include/cairo -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/pixman-1 -lcairo -lm -g
	./kostex
	viewnior kostex_petal.svg
