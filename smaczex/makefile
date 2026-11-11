default:
	gcc -o smaczex smaczex.c -I/usr/include/cairo -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/pixman-1 -lcairo -lm -g
	./smaczex -w 130 -h 110
	ls *.svg | xargs -I ^ -P 4 ./process.sh ^

test: 
	gcc -o smaczex smaczex.c -I/usr/include/cairo -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/pixman-1 -lcairo -lm -g
	./smaczex -w 125 -h 110
	viewnior smaczex_side.svg
