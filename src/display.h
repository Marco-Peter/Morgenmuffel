#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>

typedef void (*display_command_t)(void);

int display_init(void);

void display_command(display_command_t cmd);

/* Standard display commands */
void display_clear(void);
void display_aus(void);

#endif /* DISPLAY_H */
