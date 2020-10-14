#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>

typedef void (*display_command_t)(void);

int display_init(void);
int display_off(void);

void display_command(display_command_t cmd);

/* Standard display commands */
void display_clear(void);

#endif /* DISPLAY_H */
