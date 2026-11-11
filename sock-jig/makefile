default:
	gcc -o sock-jig sock-jig.c -I/usr/include/cairo -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/pixman-1 -lcairo -lm -g
	./sock-jig -w 95 -h 400
	./process.sh sock-jig.svg

test: 
	gcc -o sock-jig sock-jig.c -I/usr/include/cairo -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/pixman-1 -lcairo -lm -g
	./sock-jig -w 95 -h 400
	viewnior sock-jig.svg
