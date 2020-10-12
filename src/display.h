#ifndef DISPLAY_H
#define DISPLAY_H

typedef void (*display_command_t)(void);

int display_init(void);
int display_off(void);
void display_command(display_command_t cmd);

#endif /* DISPLAY_H */
