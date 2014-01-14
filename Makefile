all:
	clang mines.c -o mines -g -lSDL2 -lSDL2_ttf -lm

clean:
	rm mines
