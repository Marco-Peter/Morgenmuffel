#include <zephyr.h>
#include <sys/printk.h>
#include <vs1053.h>

void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
}
