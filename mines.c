#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>

SDL_Window *g_window;
SDL_Renderer *g_renderer;
SDL_Surface *g_screen;

const int32_t MINEFIELD_WIDTH = 8;
const int32_t MINEFIELD_HEIGHT = 10;
const int32_t MINE_COUNT = MINEFIELD_WIDTH * MINEFIELD_HEIGHT;

const int32_t SCREEN_WIDTH = 256;
const int32_t SCREEN_HEIGHT = 320;

const int32_t BUTTON_SIZE = 32;
const int32_t BUTTON_BORDER = 1;

bool init();
bool load_content();
void cleanup();

typedef struct mineblock_t {
	bool safe;
	bool flagged;
	bool opened;
	int32_t adjacent_mines;
} mineblock_t;

typedef struct minefield_t {
	mineblock_t blocks[MINEFIELD_WIDTH][MINEFIELD_HEIGHT];
} minefield_t;

minefield_t *minefield_new(int32_t w, int32_t h, int32_t mineCount);
bool minefield_position_valid(minefield_t *self, int32_t x, int32_t y);
mineblock_t *minefield_block(minefield_t *self, int32_t x, int32_t y);

typedef struct mouse_t {
	int32_t x;
	int32_t y;
	bool clickDown;
} mouse_t;

mouse_t g_mouse;

typedef struct color_t {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} color_t;

typedef enum {
	STATE_UP,
	STATE_DOWN,
	STATE_HOVER
} button_state_e;

typedef struct button_t {
	bool enabled;
	button_state_e state;
	SDL_Rect rectangle;
} button_t;

button_t buttons[MINE_COUNT];

button_t *button_new(int32_t x, int32_t y, int32_t w, int32_t h);
void button_render(button_t *self);
bool button_contains(button_t *self, int32_t x, int32_t y);
void button_update_state(button_t *self, mouse_t mouse);

int main(int argc, char *argv[]) {
	if (!init()) {
		return 1;
	}

	if (!load_content()) {
		return 1;
	}

	int b = 0;
	for (int i = 0; i < MINEFIELD_WIDTH; ++i) {
		for (int j = 0; j < MINEFIELD_HEIGHT; ++j) {
			buttons[b++] = *button_new(i * 32,
				j * 32,
				BUTTON_SIZE,
				BUTTON_SIZE);
		}
	}

	g_mouse.x = 0;
	g_mouse.y = 0;
	g_mouse.clickDown = false;

	bool running = true;
	SDL_Event sdlEvent;

	while (running) {
		while (SDL_PollEvent(&sdlEvent) != 0) {
			switch (sdlEvent.type) {
				case SDL_QUIT:
					running = false;
					break;

				case SDL_KEYDOWN:
					switch (sdlEvent.key.keysym.sym) {
						case SDLK_ESCAPE:
							running = false;
							break;
					}
					break;

				case SDL_MOUSEMOTION:
					SDL_GetMouseState(&(g_mouse.x), &(g_mouse.y));
					break;

				case SDL_MOUSEBUTTONDOWN:
					SDL_GetMouseState(&(g_mouse.x), &(g_mouse.y));

					if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
						g_mouse.clickDown = true;
					}
					break;

				case SDL_MOUSEBUTTONUP:
					SDL_GetMouseState(&(g_mouse.x), &(g_mouse.y));

					if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
						g_mouse.clickDown = false;
					}
					break;
			}
			if (sdlEvent.type == SDL_QUIT) {
				
			} else if (sdlEvent.type == SDL_KEYDOWN) {
				
			}
		}

		SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
		SDL_RenderClear(g_renderer);

		for (int button = 0; button < MINE_COUNT; ++button) {
			button_update_state(&buttons[button], g_mouse);
			button_render(&buttons[button]);
		}		

		SDL_RenderPresent(g_renderer);
	}

	cleanup();

	return 0;
}

bool init() {
	srand(time(NULL));

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL coult not be initialized, SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	g_window = SDL_CreateWindow("Mines", 
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN);

	if (!g_window) {
		printf("Window could not be created, SDL_Error: %s\n", 
			SDL_GetError());
		return false;			
	}

	g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
	if (!g_renderer) {
		printf("Renderer could not be created, SDL_Error: %s\n",
			SDL_GetError());
		return false;
	}

	g_screen = SDL_GetWindowSurface(g_window);

	return true;
}

bool load_content() {
	return true;
}

void cleanup() {
	SDL_DestroyWindow(g_window);
	g_window = NULL;

	SDL_Quit();
}

minefield_t *minefield_new(int32_t w, int32_t h, int32_t mineCount) {
	minefield_t *minefield = (minefield_t *)malloc(sizeof(minefield_t));

	//init blocks
	for (int32_t x = 0; x < MINEFIELD_WIDTH; ++x) {
		for (int32_t y = 0; y < MINEFIELD_HEIGHT; ++y) {
			minefield->blocks[x][y].safe = true;
			minefield->blocks[x][y].flagged = false;
			minefield->blocks[x][y].opened = false;
			minefield->blocks[x][y].adjacent_mines = 0;
		}
	}

	while (mineCount > 0) {
		int32_t rx = rand() % MINEFIELD_WIDTH;
		int32_t ry = rand() % MINEFIELD_HEIGHT;
		if (minefield->blocks[rx][ry].safe) {
			minefield->blocks[rx][ry].safe = false;
			--mineCount;
		}
	}

	return minefield;
}

bool minefield_position_valid(minefield_t *self, int32_t x, int32_t y) {
	return !(x < 0 || x >= MINEFIELD_WIDTH ||
		y < 0 || y >= MINEFIELD_HEIGHT);
}

mineblock_t *minefield_block(minefield_t *self, int32_t x, int32_t y) {
	if (!minefield_position_valid(self, x, y)) {
		return NULL;
	}

	return &self->blocks[x][y];
}

button_t *button_new(int32_t x, int32_t y, int32_t w, int32_t h) {
	button_t *newButton = (button_t *)malloc(sizeof(button_t));
	newButton->enabled = true;
	newButton->state = STATE_UP;
	newButton->rectangle.x = x + BUTTON_BORDER;
	newButton->rectangle.y = y + BUTTON_BORDER;
	newButton->rectangle.w = w - BUTTON_BORDER * 2;
	newButton->rectangle.h = h - BUTTON_BORDER * 2;

	return newButton;
}

void button_render(button_t *self) {
	color_t color = {
		0, 0, 0, 255
	};

	switch (self->state) {
		default:
		case STATE_UP:
			color.r = 200;
			color.g = 200;
			color.b = 200;
			break;

		case STATE_DOWN:
			color.r = 50;
			color.g = 50;
			color.b = 50;
			break;

		case STATE_HOVER:
			color.r = 125;
			color.g = 125;
			color.b = 125;
			break;
	}

	SDL_SetRenderDrawColor(g_renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(g_renderer, &self->rectangle);
}

bool button_contains(button_t *self, int32_t x, int32_t y) {
	return !(x < self->rectangle.x ||
			 x > self->rectangle.x + self->rectangle.w ||
			 y < self->rectangle.y ||
			 y > self->rectangle.y + self->rectangle.h);
}

void button_update_state(button_t *self, mouse_t mouse) {
	if (!button_contains(self, mouse.x, mouse.y)) {
		self->state = STATE_UP;
	} else {
		if (mouse.clickDown) {
			self->state = STATE_DOWN;
		} else {
			self->state = STATE_HOVER;
		}
	}
}