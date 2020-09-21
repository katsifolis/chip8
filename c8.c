#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define BUFF_LEN 100000
// Globals
SDL_Window   *w;
SDL_Renderer *r;
bool running  = true;
bool draw_f   = false;

const unsigned char fontset[80] = { 0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80  };

// Structs

typedef struct {
	unsigned char mem[4096];
	unsigned char V[16];
	unsigned char gfx[64*32];
	unsigned short op;
	unsigned short I;
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
			c->V[(op & 0x0F00) >> 8] = c->V[(op & 0x00F) >> 4]; 
			c->pc += 2;
			break;
		case 0x0001:
			c->V[(op & 0x0F00) >> 8] = \
				c->V[(op & 0x0F00) >> 8] | c->V[(op & 0x00F0) >> 4];
			c->pc += 2;
			break;
		case 0x0002:
			c->V[(op & 0x0F00) >> 8] = \
				c->V[(op & 0x0F00) >> 8] & c->V[(op & 0x00F0) >> 4];
			c->pc += 2;
			break;
		case 0x0003:
			c->V[(op & 0x0F00) >> 8] = \
			   c->V[(op & 0x0F00) >> 8] ^ c->V[(op & 0x00F0) >> 4];
			c->pc += 2;
			break;
		case 0x0004:
			c->V[0xF] =	c->V[(op & 0x00F0) >> 4] > 0xFF - c->V[(op & 0x0F00) >> 8] ? 1 :  0;
			c->V[(op & 0x0F00) >> 8] += c->V[(op & 0x00F0) >> 4];
			c->pc += 2;
			break;
		case 0x0005:
			c->V[0xF] =	(c->V[(op & 0x00F0) >> 4] > c->V[(op & 0x0F00) >> 8]) ? 0 :  1;
			c->V[(op & 0x0F00) >> 8] -= c->V[(op & 0x00F0) >> 4];
			c->pc += 2;
			break;

		case 0x0006:
			c->V[0xF] = c->V[(op & 0x0F00) >> 8] & 0x01 ? 1 : 0;
			c->V[(op & 0x0F00) >> 8] >>= 1;
			c->pc += 2;
			break;
		case 0x0007:
			c->V[0xF] =	(c->V[(op & 0x0F00) >> 8] > c->V[(op & 0x00F0) >> 4]) ? 0 :  1;
			c->V[(op & 0x00F0) >> 4] -= c->V[(op & 0x0F00) >> 8];
			c->pc += 2;
			break;
		case 0x000E:
			c->V[0xF] = c->V[(op & 0x0F00) >> 8] & 0x10 ? 1 : 0;
			c->V[(op & 0x0F00) >> 8] <<= 1;
			c->pc += 2;
			break;
		}
	case 0x9000:
		if (c->V[(op & 0x0F00) >> 8] != c->V[(op & 0x00F0) >> 4]) {
			c->pc +=2;
		}
		c->pc += 2;
		break;
	case 0xA000:
		c->I  = op & 0x0FFF;
		c->pc += 2;
		break;

	case 0xB000:
		c->pc = c->V[0x0] + (op & 0x0FFF);
		break;

	case 0xC000:
		c->V[(op & 0x0F00) >> 8] = \
			(rand() % 255) & (op & 0xFF);
		c->pc += 2;
		break;

	case 0XD000:
	{
		unsigned short x = c->V[(op & 0x0F00) >> 8];
		unsigned short y = c->V[(op & 0x00F0) >> 4];
		unsigned short h = op & 0x000F;
		unsigned short p;

		c->V[0xF] = 0;
		for (int yline = 0; yline < h; yline++) {
			p = c->mem[c->I + yline];
			for (int xline = 0; xline < 8; xline++) {
				if((p & (0x80 >> xline)) != 0) {
					if(c->gfx[(x + xline + ((y + yline) * 64))] == 1)
						c->V[0xF] = 1;                                 
					c->gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}
		draw_f = true;
		c->pc += 2;
	}
		break;

	case 0xE000:
		switch (op & 0x00FF) {
		case 0x009E: c->pc += (c->key[c->V[(op & 0x0F00) >> 8]] != 0) ? 4 : 2; break;
		case 0x00A1: c->pc += (c->key[c->V[(op & 0x0F00) >> 8]] == 0) ? 4 : 2; break;
		}
		break;

	case 0xF000:
		switch (op & 0x00FF) {
		case 0x0007: c->V[(op & 0x0F00) >> 8] = c->dt; c->pc += 2;
			
		case 0x000A: 
		{
			bool keypress = false;
			for (int i = 0; i < 16; i++) {
				if (c->key[i] != 0) {
					c->V[(op &0x0F00) >> 8] = c->key[i];
					keypress = true;
				}
			}
			if (!keypress) return;

			c->pc += 2;
		}
		break;

		case 0x0015: c->dt = c->V[(op & 0x0F00) >> 8]; c->pc += 2; break;
		case 0x0018: c->st = c->V[(op & 0x0F00) >> 8]; c->pc += 2; break;
		case 0x001E: c->I += c->V[(op & 0x0F00) >> 8]; c->pc += 2; break;
		case 0x0029: c->I += c->V[(op & 0x0F00) >> 8]; c->pc += 2; break;
		case 0x0033: 
			c->mem[c->I]     = c->V[(op & 0x0F00) >> 8] / 100;
			c->mem[c->I + 1] = (c->V[(op & 0x0F00) >> 8] / 10) % 10;
			c->mem[c->I + 2] = (c->V[(op & 0x0F00) >> 8] % 100) % 10;
			c->pc += 2;
		break;
		case 0x0055:
			for (int i = 0; i < c->V[(op & 0x0F00) >>8]; i++) {
				c->mem[c->I + i] = c->V[i];
			}
			c->pc += 2;
			break;
		case 0x0065:
			for (int i = 0; i < c->V[(op & 0x0F00) >>8]; i++) {
				c->V[i] = c->mem[c->I + i];
			}
			c->pc += 2;
			break;

		}
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


	SDL_SetRenderDrawColor(r, 0x00, 0x00, 0x00, 0xff);

	return 0;


}


void event(CHIP8 *c) {
	SDL_Event e;
	if (SDL_WaitEvent(&e) != 0) {
		switch (e.type) {
		case SDL_QUIT:
			running = false;
			break;

		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {

			case SDLK_ESCAPE: running = false; break;

			case SDLK_1: 	c->key[0x1] = 1; break; 
			case SDLK_2:	c->key[0x2] = 1; break;
			case SDLK_3:	c->key[0x3] = 1; break;
			case SDLK_4:	c->key[0xC] = 1; break;

			case SDLK_q:	c->key[0x4] = 1; break;
			case SDLK_w:	c->key[0x5] = 1; break;
			case SDLK_e:	c->key[0x6] = 1; break;
			case SDLK_r:	c->key[0xD] = 1; break;

			case SDLK_a:	c->key[0x7] = 1; break;
			case SDLK_s:	c->key[0x8] = 1; break;
			case SDLK_d:	c->key[0x9] = 1; break;
			case SDLK_f:	c->key[0xE] = 1; break;

			case SDLK_z:	c->key[0xA] = 1; break;
			case SDLK_x:	c->key[0x0] = 1; break;
			case SDLK_c:	c->key[0xB] = 1; break;
			case SDLK_v:	c->key[0xF] = 1; break;

			}
			break;
		case SDL_KEYUP:
			switch (e.key.keysym.sym) {
			case SDLK_1:	c->key[0x1] = 0; break;   
			case SDLK_2:	c->key[0x2] = 0; break;
			case SDLK_3:	c->key[0x3] = 0; break;
			case SDLK_4:	c->key[0xC] = 0; break;

			case SDLK_q:	c->key[0x4] = 0; break;
			case SDLK_w:	c->key[0x5] = 0; break;
			case SDLK_e:	c->key[0x6] = 0; break;
			case SDLK_r:	c->key[0xD] = 0; break;

			case SDLK_a:	c->key[0x7] = 0; break;
			case SDLK_s:	c->key[0x8] = 0; break;
			case SDLK_d:	c->key[0x9] = 0; break;
			case SDLK_f:	c->key[0xE] = 0; break;

			case SDLK_z:	c->key[0xA] = 0; break;
			case SDLK_x:	c->key[0x0] = 0; break;
			case SDLK_c:	c->key[0xB] = 0; break;
			case SDLK_v:	c->key[0xF] = 0; break;
			break;
			}
		}
	}
}

void update(CHIP8 *c) {
	SDL_Rect rect = {0, 0, 8, 0};

	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 64; x++) {
			if (c->gfx[y * 32 + x] != 0) {
				rect.x = x;
				rect.y = y;
				continue;
			}

		}
	}
	return;


}
int
main()
{
	CHIP8 b = new_chip8();
	srand(time(NULL));
	emulate_cycle(&b);

	sdl_setup();
	while(running) {
		event(&b);
		if (draw_f) SDL_RenderClear(r);
		SDL_RenderPresent(r);
	}


	SDL_DestroyRenderer(r);
	SDL_DestroyWindow(w);
	SDL_Quit();
	return 0;
}
