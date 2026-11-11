default:
	gcc -o bateriex bateriex.c -I/usr/include/cairo -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/pixman-1 -lcairo -lm -g
	./bateriex -w 45 -l 150
	ls *.svg | xargs -I ^ -P 4 ./process.sh ^

test: 
	gcc -o bateriex bateriex.c -I/usr/include/cairo -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/pixman-1 -lcairo -lm -g
	./bateriex -w 45 -l 145
	viewnior *.svg
