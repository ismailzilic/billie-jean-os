#include "kernel.h"
#include "idt/idt.h"
#include "io/io.h"

#include <stddef.h>
#include <stdint.h>

uint16_t *video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

uint16_t terminal_make_char(char c, char color) { return (color << 8) | c; }

void terminal_putchar(int x, int y, char c, char color) { video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, color); }

void terminal_write_char(char c, char color)
{
	if (c == '\n') {
		terminal_row++;
		terminal_col = 0;
		return;
	}

	terminal_putchar(terminal_col, terminal_row, c, color);

	terminal_col++;
	if (terminal_col >= VGA_WIDTH) {
		terminal_col = 0;
		terminal_row = 1;
	}
}

void terminal_initialize()
{
	video_mem = (uint16_t *)(0xB8000);

	for (int y = 0; y < VGA_HEIGHT; ++y) {
		for (int x = 0; x < VGA_WIDTH; ++x) {
			terminal_putchar(x, y, ' ', 0);
		}
	}
}

size_t strlen(const char *str)
{
	size_t length = 0;

	while (str[length])
		length++;

	return length;
}

void print(const char *str)
{
	size_t len = strlen(str);

	for (int i = 0; i < len; i++)
		terminal_write_char(str[i], 15);
}

void kernel_main()
{
	terminal_initialize();
	print("ZEUSSS! Is this how you face me, coward!\n");

	idt_init();

	outb(0x60, 0xFF);

	// Enable system interrupts
	enable_interrupts();
}
