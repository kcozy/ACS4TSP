# MakeFile for TSP
myTsp:tspShow.c main.c
	gcc -O2 -I/usr/include/X11/ -I/opt/X11/include -L/usr/lib/ -L/usr/X11R6/lib -lX11 -lm tspShow.c main.c -o main