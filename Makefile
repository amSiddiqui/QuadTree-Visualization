quadtree: quadtree.c
	gcc -Wall -lSDL2 quadtree.c -o QT

clean: quadtree
	rm quadtree
