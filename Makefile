quadtree: quadtree.c
	gcc -Wall quadtree.c -o quadtree

standalone: quadtree_sdl.c
	gcc -Wall -lSDL2 quadtree_sdl.c -o QT_SDL

clean: quadtree
	rm quadtree
