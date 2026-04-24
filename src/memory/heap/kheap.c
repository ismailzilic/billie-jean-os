#include "kheap.h"
#include "../../kernel.h"
#include "heap.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

void kheap_init()
{
	int total_table_entries = HEAP_SIZE_BYTES / HEAP_BLOCK_SIZE;
	kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY *)(HEAP_TABLE_ADDRESS);
	kernel_heap_table.totalent = total_table_entries;

	void *end = (void *)(HEAP_ADDRESS + HEAP_SIZE_BYTES);
	int res = heap_create(&kernel_heap, &kernel_heap_table, (void *)(HEAP_ADDRESS), end);

	if (res < 0) {
		print("Failed to create heap\n");
	}
}

void *kmalloc(size_t size) { return heap_malloc(&kernel_heap, size); }

void kfree(void *ptr) { heap_free(&kernel_heap, ptr); }
