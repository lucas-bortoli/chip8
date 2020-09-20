hellomake: main.c
	gcc -o main main.c `sdl2-config --cflags --libs`
