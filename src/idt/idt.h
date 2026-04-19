#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_desc {
	uint16_t offest_1; // Offest bits 0 - 15
	uint16_t selector; // Selector in GDT
	uint8_t zero;	   // Set to zero
	uint8_t type_attr; // Desc type and attr
	uint16_t offest_2; // Offset bits 16 - 31
} __attribute((packed));

struct idtr_desc {
	uint16_t limit; // Size of desc table - 1
	uint32_t base;	// Base address of the start of the IDT
} __attribute((packed));

void idt_init();

#endif // IDT_H
