#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include <SDL2/SDL.h>

SDL_Window *g_window;
SDL_Renderer *g_renderer;
SDL_Surface *g_screen;

#define MINEFIELD_WIDTH 8
#define MINEFIELD_HEIGHT 10
#define MINEFIELD_COUNT (MINEFIELD_WIDTH * MINEFIELD_HEIGHT)

const int32_t SCREEN_WIDTH = 256;
const int32_t SCREEN_HEIGHT = 320;

const int32_t BUTTON_SIZE = 32;
const int32_t BUTTON_BORDER = 1;

bool init();
bool load_content();
void cleanup();

int32_t xy_to_index(int32_t x, int32_t y, int32_t w, int32_t h);
void index_to_xy(int32_t index, int32_t w, int32_t h, int32_t *x, int32_t *y);

typedef struct mineblock_t {
	bool safe;
	bool flagged;
	bool opened;
	int32_t adjacent_mines;
} mineblock_t;

typedef struct minefield_t {
	int32_t width, height, size;
	mineblock_t *blocks;
} minefield_t;

void minefield_reset(minefield_t *self, int32_t w, int32_t h);
void minefield_seed_mines(minefield_t *self, int32_t mineCount,
	int32_t safeX, int32_t safeY);
bool minefield_reveal(minefield_t *self, int32_t x, int32_t y);
int32_t minefield_get_adjacent_count(minefield_t *self, int32_t x, int32_t y);
bool minefield_position_valid(minefield_t *self, int32_t x, int32_t y);
mineblock_t *minefield_block(minefield_t *self, int32_t x, int32_t y);

typedef struct mouse_t {
	int32_t x;
	int32_t y;
	bool clickDown;

	int32_t selectedX;
	int32_t selectedY;
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
	int32_t gx;
	int32_t gy;
	button_state_e state;
	SDL_Rect rectangle;

	mineblock_t *data;
} button_t;

button_t *buttons;

button_t *button_new(int32_t x, int32_t y, int32_t w, int32_t h);
void button_reset(int32_t x, int32_t y, int32_t w, int32_t h, button_t *dest);
void button_render(button_t *self);
bool button_contains(button_t *self, int32_t x, int32_t y);
void button_update_state(button_t *self, mouse_t *mouse);

int main(int argc, char *argv[]) {
	if (!init()) {
		return 1;
	}

	if (!load_content()) {
		return 1;
	}

	minefield_t minefield;
	minefield_reset(&minefield, MINEFIELD_WIDTH, MINEFIELD_HEIGHT);

	buttons = (button_t *)malloc(sizeof(button_t) * MINEFIELD_COUNT);

	int b = 0;
	for (int i = 0; i < MINEFIELD_WIDTH; ++i) {
		for (int j = 0; j < MINEFIELD_HEIGHT; ++j) {
			button_reset(
				i * 32, 
				j * 32,
				BUTTON_SIZE,
				BUTTON_SIZE,
				&buttons[b]
			);
			buttons[b].gx = i;
			buttons[b].gy = j;
			buttons[b].data = &minefield.blocks[b];
			++b;
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

						printf("%d, %d - %d %d\n", 
							g_mouse.x,
							g_mouse.y,
							g_mouse.selectedX,
							g_mouse.selectedY);

						minefield_reveal(&minefield, 
							g_mouse.selectedX,
							g_mouse.selectedY);
					}
					break;
			}
			if (sdlEvent.type == SDL_QUIT) {
				
			} else if (sdlEvent.type == SDL_KEYDOWN) {
				
			}
		}

		SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
		SDL_RenderClear(g_renderer);

		for (int button = 0; button < MINEFIELD_COUNT; ++button) {
			button_update_state(&buttons[button], &g_mouse);
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

int32_t xy_to_index(int32_t x, int32_t y, int32_t w, int32_t h) {
	if (x < 0 || y < 0 || x >= w || y >= h) {
		return -1;
	}
	return x * h + y;
}

void index_to_xy(int32_t index, int32_t w, int32_t h, int32_t *x, int32_t *y) {
	if (index < 0 || index >= (w * h)) {
		*x = -1;
		*y = -1;
		return;
	}

	*x = index % w;
	*y = floor(index / w);
}

void minefield_reset(minefield_t *self, int32_t w, int32_t h) {
	self->width = w;
	self->height = h;
	self->size = w * h;
	self->blocks = (mineblock_t *)malloc(sizeof(mineblock_t) *
										 self->size);

	//init blocks
	for (int32_t b = 0; b < self->size; ++b) {
		self->blocks[b].safe = true;
		self->blocks[b].flagged = false;
		self->blocks[b].opened = false;
		self->blocks[b].adjacent_mines = 0;
	}
}

void minefield_seed_mines(minefield_t *self, int32_t mineCount,
	int32_t safeX, int32_t safeY) {
	while (mineCount > 0) {
		int32_t rx = rand() % self->width;
		int32_t ry = rand() % self->height;

		if (rx == safeX && ry == safeY) {
			continue;
		}

		int32_t r = xy_to_index(rx, ry, self->width, self->height);
		if (self->blocks[r].safe) {
			self->blocks[r].safe = false;
			--mineCount;
		}
	}
}

bool minefield_reveal(minefield_t *self, int32_t x, int32_t y) {
	if (!minefield_position_valid(self, x, y)) {
		return true;
	}

	int32_t index = xy_to_index(x, y, self->width, self->height);
	if (self->blocks[index].safe) {
		self->blocks[index].opened = true;
		self->blocks[index].adjacent_mines = minefield_get_adjacent_count(self,
			x,
			y);
	}
}

int32_t minefield_get_adjacent_count(minefield_t *self, int32_t x, int32_t y) {
	int32_t tlx = x - 1;
	int32_t tly = y - 1;

	int32_t count = 0;

	int32_t indices[8];
	indices[0] = xy_to_index(tlx + 0, tly + 0, self->width, self->height);
	indices[1] = xy_to_index(tlx + 1, tly + 0, self->width, self->height);
	indices[2] = xy_to_index(tlx + 2, tly + 0, self->width, self->height);

	indices[3] = xy_to_index(tlx + 0, tly + 1, self->width, self->height);
	indices[4] = xy_to_index(tlx + 2, tly + 1, self->width, self->height);

	indices[5] = xy_to_index(tlx + 0, tly + 2, self->width, self->height);
	indices[6] = xy_to_index(tlx + 1, tly + 2, self->width, self->height);
	indices[7] = xy_to_index(tlx + 2, tly + 2, self->width, self->height);

	for (int32_t i = 0; i < 8; ++i) {
		int32_t index = indices[i];
		if (index >= 0) {
			if (!self->blocks[index].safe) {
				++count;
			}
		}
	}

	return count;
}

bool minefield_position_valid(minefield_t *self, int32_t x, int32_t y) {
	return !(x < 0 || x >= MINEFIELD_WIDTH ||
		y < 0 || y >= MINEFIELD_HEIGHT);
}

mineblock_t *minefield_block(minefield_t *self, int32_t x, int32_t y) {
	if (!minefield_position_valid(self, x, y)) {
		return NULL;
	}

	return &self->blocks[xy_to_index(x, y, self->width, self->height)];
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

void button_reset(int32_t x, int32_t y, int32_t w, int32_t h, button_t *dest) {
	dest->enabled = true;
	dest->state = STATE_UP;
	dest->rectangle.x = x + BUTTON_BORDER;
	dest->rectangle.y = y + BUTTON_BORDER;
	dest->rectangle.w = w - BUTTON_BORDER * 2;
	dest->rectangle.h = h - BUTTON_BORDER * 2;
}

void button_render(button_t *self) {
	color_t color = {
		0, 0, 0, 255
	};

	if (!self->data->opened) {
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
	} else {
		color.r = 255;
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

void button_update_state(button_t *self, mouse_t *mouse) {
	if (!button_contains(self, mouse->x, mouse->y)) {
		self->state = STATE_UP;
	} else {
		if (mouse->clickDown) {
			mouse->selectedX = self->gx;
			mouse->selectedY = self->gy;
			self->state = STATE_DOWN;
		} else {
			self->state = STATE_HOVER;
		}
	}
}