#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// Globals

SDL_Window   *w;
SDL_Renderer *r;
bool quit    = false;


#define BUFF_LEN 100000
const unsigned char fontset[80] = {
			0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			0x20, 0x60, 0x20, 0x20, 0x70, // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

typedef struct {
	unsigned char mem[4096];
	unsigned char reg[16];
	unsigned char gx[64*32];
	unsigned short op;
	unsigned short i;
	unsigned short pc;
	unsigned char dt;
	unsigned char st;
	unsigned short stack[16];
	unsigned short sp;
	unsigned char key[16];
	unsigned char font[80];
} CHIP8;

CHIP8 new_chip8() {

	// Initializing the system
	CHIP8 c;
	c.sp = 0x200;

	// Opening the game to read
	FILE *f = fopen("breakout.ch8", "r");
	if (f == NULL) {
		fprintf(stderr, "NOT FOUND\n");
		exit(-1);
	}

	// Deteriming the length of the file
	fseek(f, 0, SEEK_END);
	long flsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	// Allocating the buffer
	char *buffer = malloc(flsize * sizeof(char));
	int n = fread(buffer, 1, BUFF_LEN, f);
	if (n < 0) {
		fprintf(stderr, "Couldn't read into the buffer\n");
		exit(-1);
	}

	for (int i = 0; i < flsize; i++) 
		c.mem[i + 512] = buffer[i];

	return c;
}

void emulate_cycle(CHIP8 *c) {

}

int sdl_setup() {

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	w = SDL_CreateWindow("Hello World!", 100, 100, 800, 600, SDL_WINDOW_RESIZABLE);
	if (w == NULL) {
		fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	r = SDL_CreateRenderer(w, -1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (r == NULL) {
		fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
		if (w != NULL) {
			SDL_DestroyWindow(w);
		}
		SDL_Quit();
		return EXIT_FAILURE;
	}


}


void event() {
	SDL_Event e;
	if (SDL_WaitEvent(&e) != 0) {
		switch (e.type) {
		case SDL_QUIT:
			quit = true;
			break;
		}
	}
}

int
main()
{
	sdl_setup();
	CHIP8 a = new_chip8();

	SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
	while(!quit) {
		event();
		SDL_RenderClear(r);
		SDL_RenderPresent(r);
	}

	SDL_DestroyRenderer(r);
	SDL_DestroyWindow(w);
	SDL_Quit();
	return 0;
}
