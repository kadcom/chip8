#ifndef CHIP8_RENDER_H 
#define CHIP8_RENDER_H

#include "chip8.h"
struct render_t;

int create_renderer(struct render_t **renderer);
int initialise_renderer(struct render_t *renderer, void *window, int width, int height);
int render_display(struct render_t *renderer, struct machine_t *machine);
int destroy_renderer(struct render_t *renderer);
#endif 
