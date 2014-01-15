#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

SDL_Window *g_window;
SDL_Renderer *g_renderer;
SDL_Surface *g_screen;

TTF_Font *g_font;

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

typedef struct strtexture_t {
	char *text;
	int32_t width;
	int32_t height;
	SDL_Texture *texture;
} strtexture_t;

strtexture_t *strtexture_new(const char *text);
void strtexture_render(strtexture_t *self,
	int32_t x,
	int32_t y,
	SDL_Color *color);

#define MAX_STR_TEXTURES 20
strtexture_t *g_strTextures[MAX_STR_TEXTURES];
int32_t g_strTexIndex;

void strtextures_init();
void strtextures_add(const char *text);
strtexture_t *strtextures_get(const char *text);

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
void minefield_reveal(minefield_t *self, int32_t x, int32_t y);
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

SDL_Color g_colorRed = 		{ 255,   0,   0, 255 };
SDL_Color g_colorGreen = 	{   0, 255,   0, 255 };
SDL_Color g_colorBlue = 	{   0,   0, 225, 255 };
SDL_Color g_colorCyan = 	{   0, 225, 225, 255 };
SDL_Color g_colorMagenta = 	{ 225,   0, 225, 255 };
SDL_Color g_colorYellow = 	{ 225, 225,   0, 255 };
SDL_Color g_colorWhite =	{ 255, 255, 255, 255 };
SDL_Color g_colorBlack =	{   0,   0,   0, 255 };

int main(int argc, char *argv[]) {
	if (!init()) {
		return 1;
	}

	if (!load_content()) {
		return 1;
	}

	minefield_t minefield;
	minefield_reset(&minefield, MINEFIELD_WIDTH, MINEFIELD_HEIGHT);
	minefield_seed_mines(&minefield, 10, 0, 0);

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

	strtextures_init();
	strtextures_add("0");
	strtextures_add("1");
	strtextures_add("2");
	strtextures_add("3");
	strtextures_add("4");
	strtextures_add("5");
	strtextures_add("6");
	strtextures_add("7");
	strtextures_add("8");

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

				button_t *current;

				for (int b = 0; b < MINEFIELD_COUNT; ++b) {
					if (button_contains(&buttons[b],
										g_mouse.x,
										g_mouse.y)) {
						current = &buttons[b];
					}
				}				

				if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
					g_mouse.clickDown = true;

					if (!current->data->flagged) {
						g_mouse.selectedX = current->gx;
						g_mouse.selectedY = current->gy;
					}					
				} else if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
					current->data->flagged = !current->data->flagged;
				}
				break;

			case SDL_MOUSEBUTTONUP:
				SDL_GetMouseState(&(g_mouse.x), &(g_mouse.y));

				if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
					g_mouse.clickDown = false;


					int32_t index = xy_to_index(g_mouse.selectedX,
						g_mouse.selectedY,
						minefield.width,
						minefield.height);

					if (button_contains(&buttons[index],
						g_mouse.x, g_mouse.y)) {
						minefield_reveal(&minefield, 
							g_mouse.selectedX,
							g_mouse.selectedY);
					}
				}
				break;
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

	if (TTF_Init() == -1) {
		printf("SDL_ttf could not initialize, SDL_ttf Error: %s\n",
			TTF_GetError());
		return false;
	}

	return true;
}

bool load_content() {
	g_font = TTF_OpenFont("pstart2p.ttf", 12);
	if (g_font == NULL) {
		printf("Could not load font, SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

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

strtexture_t *strtexture_new(const char *text) {
	SDL_Color color = {
		255, 255, 255, 255
	};

	SDL_Surface *textSurface = TTF_RenderText_Solid(g_font,
		text,
		color);

	if (textSurface == NULL) {
		printf("Unable to render text surface! SDL_ttf Error: %s\n",
			TTF_GetError());
		return NULL;
	}

	strtexture_t *strtexture = (strtexture_t *)malloc(sizeof(strtexture_t));
	strtexture->text = (char *)malloc(sizeof(char) * strlen(text));
	strcpy(strtexture->text, text);
	strtexture->width = textSurface->w;
	strtexture->height = textSurface->h;
	strtexture->texture = SDL_CreateTextureFromSurface(g_renderer, textSurface);

	SDL_FreeSurface(textSurface);

	return strtexture;
}

void strtexture_render(strtexture_t *self,
	int32_t x,
	int32_t y,
	SDL_Color *color) {

	if (color != NULL) {
		SDL_SetTextureColorMod(self->texture,
			color->r,
			color->g,
			color->b);
	} else {
		SDL_SetTextureColorMod(self->texture,
			255,
			255,
			255);
	}

	SDL_Rect renderRect = { x, y, self->width, self->height };

	SDL_RenderCopyEx(g_renderer, self->texture, NULL, &renderRect, 0, NULL, 0);
}

void strtextures_init() {
	g_strTexIndex = 0;
	for (int i = 0; i < MAX_STR_TEXTURES; ++i) {
		g_strTextures[i] = NULL;
	}
}

void strtextures_add(const char *text) {
	if (g_strTexIndex >= MAX_STR_TEXTURES) {
		printf("Error: max size of str texture array reached\n");
		return;
	}

	g_strTextures[g_strTexIndex] = strtexture_new(text);
	++g_strTexIndex;
}

strtexture_t *strtextures_get(const char *text) {
	for (int i = 0; i < g_strTexIndex; ++i) {
		char *t = g_strTextures[i]->text;
		if (strcmp(t, text) == 0) {
			return g_strTextures[i];
		}
	}

	return NULL;
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

void minefield_reveal(minefield_t *self, int32_t x, int32_t y) {
	if (!minefield_position_valid(self, x, y)) {
		return;
	}

	int32_t index = xy_to_index(x, y, self->width, self->height);

	if (self->blocks[index].opened) {
		return;
	}
	
	if (self->blocks[index].safe) {
		self->blocks[index].opened = true;
		int32_t adjacent = minefield_get_adjacent_count(self,
			x,
			y);
		self->blocks[index].adjacent_mines = adjacent;
		if (adjacent == 0) {
			minefield_reveal(self, x - 1, y - 1);
			minefield_reveal(self, x + 0, y - 1);
			minefield_reveal(self, x + 1, y - 1);
			minefield_reveal(self, x - 1, y + 0);
			minefield_reveal(self, x + 1, y + 0);
			minefield_reveal(self, x - 1, y + 1);
			minefield_reveal(self, x + 0, y + 1);
			minefield_reveal(self, x - 1, y + 1);
		}
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
	SDL_Color color = {
		0, 0, 0, 255
	};

	bool renderNumber = false;

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
		color.r = 220;
		color.g = 220;
		color.b = 220;

		renderNumber = true;
	}

	SDL_SetRenderDrawColor(g_renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(g_renderer, &self->rectangle);

	if (renderNumber) {
		char name[5];
		sprintf(name, "%d", self->data->adjacent_mines);

		strtexture_render(strtextures_get(name),
			self->rectangle.x + 8,
			self->rectangle.y + 8,
			&g_colorBlack);
	}

	if (self->data->flagged) {
		SDL_Rect flagRect = {
			self->rectangle.x + 8,
			self->rectangle.y + 8,
			16,
			16
		};

		SDL_SetRenderDrawColor(g_renderer, 255, 0, 0, 255);
		SDL_RenderFillRect(g_renderer, &flagRect);
	}
}

bool button_contains(button_t *self, int32_t x, int32_t y) {
	return !(x < self->rectangle.x ||
			 x > self->rectangle.x + self->rectangle.w ||
			 y < self->rectangle.y ||
			 y > self->rectangle.y + self->rectangle.h);
}

void button_update_state(button_t *self, mouse_t *mouse) {
	if (self->data->flagged) {
		self->state = STATE_UP;
		return;
	}

	if (mouse->clickDown) {
		if (mouse->selectedX == self->gx && 
			mouse->selectedY == self->gy) {
			self->state = STATE_DOWN;
		} else {
			self->state = STATE_UP;
		}
	} else {
		if (button_contains(self, mouse->x, mouse->y)) {
			self->state = STATE_HOVER;
		} else {
			self->state = STATE_UP;
		}
	}
}