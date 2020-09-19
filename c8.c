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
	unsigned char V[16];
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
	c.pc = 0x200;

	// Opening the game to read
	FILE *f = fopen("s.ch8", "r");
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

	// Filling the struct with game's data
	for (int i = 0; i < flsize; i++) 
		c.mem[i + 512] = buffer[i];

	// Copying the font set to the struct
	memcpy(&c.font, &fontset, 80);

	return c;
}

void emulate_cycle(CHIP8 *c) {
	uint16_t op = c->mem[c->pc] << 8 | c->mem[c->pc + 1];
	switch (op & 0xF000) {
	case 0x0000: // Clears the screen
		SDL_RenderClear(r);
		break;
	case 0x000F:
		c->pc = c->stack[c->sp];
		c->sp--;
		break;
	case 0x1000:
		c->pc = op & 0x0FFF;
		break;
	case 0x2000:
		c->stack[c->sp] = c->pc;
		c->sp++;
		c->pc = op & 0x0FFF;
		break;
	case 0x3000:
		if (c->V[(op & 0x0F00) >> 8] == (op & 0x00FF)) c->pc += 4;
		break;
	case 0x4000:
		if (c->V[(op & 0x0F000) >> 8] != (op & 0x00FF)) c->pc += 4;
		break;
	case 0x5000:
		if (c->V[(op & 0x0F00) >> 8] == c->V[(op & 0x00F0) >> 4]) c->pc += 4;
		break;
	case 0x6000:
		c->V[(op & 0x0F00) >> 8] = (unsigned char)op; //(op & 0x00FF) >> 8;
		c->pc += 2;
		break;
	case 0x7000:
		c->V[(op & 0x0F00) >> 8] += (unsigned char)op;
		c->pc += 2;
		break;
	case 0x8000:
		switch (op & 0x000F) {
		case 0x0000:
			c->V[(op & 0x0F00) >> 8] = c->[(op & 0x00F) >> 4]; 
			c->pc += 2;
			break;
	}



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

	return 0;


}


void event() {
	SDL_Event e;
	if (SDL_WaitEvent(&e) != 0) {
		switch (e.type) {
		case SDL_QUIT:
			quit = true;
			break;

		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
			case SDLK_a:
				SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);
				break;
			case SDLK_b:
				SDL_SetRenderDrawColor(r, 0x44, 0x44, 0x44, 0xff);
				break;
			}
		}
	}
}
int
main()
{
	sdl_setup();
	CHIP8 b = new_chip8();

	emulate_cycle(&b);

	SDL_SetRenderDrawColor(r, 0x33, 0x33, 0x33, 0xff);
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
