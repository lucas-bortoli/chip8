#include <SDL.h>
#include <time.h>
#include <stdio.h>
#include "chip8.c"

const int SCALE = 8;
const int SCREEN_WIDTH = 64 * SCALE;
const int SCREEN_HEIGHT = 32 * SCALE;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

int main(int argc, char* args[]) {
	SDL_Window *window = NULL;
	SDL_Surface *screenSurface = NULL;

	int frame = 0x0;
	int frame_limit = 1000;

	Chip *c = (Chip*)malloc(sizeof(Chip));

	if (argc < 2) {
		fprintf(stderr, "Nenhuma ROM especificada!\n");
		exit(1);
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Erro inicialização SDL! %s\n", SDL_GetError());
		return 1;
	} else {
		window = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			fprintf(stderr, "Janela não pôde ser criada! %s\n", SDL_GetError());
			return 1;
		} else {

			SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

			if (renderer == NULL) {
				fprintf(stderr, "Impossível criar um renderer! %s\n", SDL_GetError());
				return 1;
			}

			// carregar uma ROM no emulador
			Chip8_Load(c, args[1]);

			// loop principal infinito
			while (1) {
				SDL_Event event;
				if (SDL_PollEvent(&event)) {
					if (event.type == SDL_QUIT) {
						// parar o loop de eventos
						break;
					} else if (event.type == SDL_KEYDOWN) {
						Chip8_Update(c);
						Chip8_HandleKeyboard(c, event.key.keysym.sym, 0x1);

						// controles do emulador
						if (event.key.keysym.sym == SDLK_RIGHTBRACKET) {
							frame_limit -= 100;
							if (frame_limit < 0) frame_limit = 0;
							printf("Frame timer: %d\n", frame_limit);
						} else if (event.key.keysym.sym == SDLK_LEFTBRACKET) {
							frame_limit += 100;
							if (frame_limit > 2000) frame_limit = 2000;
							printf("Frame timer: %d\n", frame_limit);
						}
					} else if (event.type == SDL_KEYUP) {
						Chip8_HandleKeyboard(c, event.key.keysym.sym, 0x0);

						if (event.key.keysym.sym == SDLK_ESCAPE) {
							break;
						}
					}
				}

				if (frame == 0x0) {
					frame = frame_limit;
				} else {
					frame--;
					continue;
				}

				SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderClear(renderer);

				if (c->debug_autorun == 1)
					Chip8_Update(c);
				Chip8_Render(c, renderer);

				SDL_RenderPresent(renderer);
			}

			free(c);
			SDL_DestroyWindow(window);
			SDL_Quit();
			return 0;
		}
	}
}
