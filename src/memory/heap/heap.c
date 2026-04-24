#include "heap.h"
#include "../../kernel.h"
#include "../../status.h"
#include "../memory.h"

#include <stdint.h>

static int heap_validate_alignment(void *ptr) { return ((unsigned int)ptr % HEAP_BLOCK_SIZE) == 0; }

static int heap_validate_table(struct heap_table *table, void *ptr, void *end)
{
	int res = 0;

	size_t table_size = (size_t)(end - ptr);
	size_t total_blocks = table_size / HEAP_BLOCK_SIZE;

	if (table->totalent != total_blocks) {
		res = -EINVARG;
		goto out;
	}

out:
	return res;
}

int heap_create(struct heap *heap, struct heap_table *table, void *ptr, void *end)
{
	int res = 0;

	if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end)) {
		res = -EINVARG;
		goto out;
	}

	memset(heap, 0, sizeof(struct heap));
	heap->saddr = ptr;
	heap->table = table;

	res = heap_validate_table(table, ptr, end);

	if (res < 0)
		goto out;

	size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->totalent;
	memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

out:
	return res;
}

static uint32_t heap_align_value_to_block_size(uint32_t val)
{
	if ((val % HEAP_BLOCK_SIZE) == 0)
		return val;

	val = (val - (val % HEAP_BLOCK_SIZE));
	val += HEAP_BLOCK_SIZE;

	return val;
}

static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry) { return entry & 0x0F; }

int heap_get_start_block(struct heap *heap, uint32_t total_blocks)
{
	struct heap_table *table = heap->table;

	int bc = 0;  // Current block
	int bs = -1; // Starting block

	for (size_t i = 0; i < table->totalent; ++i) {
		if (heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE) {
			bc = 0;
			bs = -1;
			continue;
		}

		if (bs == -1)
			bs = i;

		bc++;
		if (bc == total_blocks)
			break;
	}

	if (bs == -1)
		return -ENOMEM;

	return bs;
}

void *heap_block_to_address(struct heap *heap, uint32_t block) { return heap->saddr + (block * HEAP_BLOCK_SIZE); }

void heap_mark_blocks_taken(struct heap *heap, uint32_t start_block, uint32_t total_blocks)
{
	uint32_t end_block = (start_block + total_blocks) - 1;

	HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;

	if (total_blocks > 1) {
		entry |= HEAP_BLOCK_HAS_NEXT;
	}

	for (uint32_t i = start_block; i <= end_block; ++i) {
		heap->table->entries[i] = entry;
		entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;

		if (i != end_block - 1) {
			entry |= HEAP_BLOCK_HAS_NEXT;
		}
	}
}

int heap_address_to_block(struct heap *heap, void *address) { return ((int)(address - heap->saddr)) / HEAP_BLOCK_SIZE; }

void heap_mark_blocks_free(struct heap *heap, uint32_t start_block)
{
	struct heap_table *table = heap->table;

	for (uint32_t i = start_block; i < (uint32_t)table->totalent; ++i) {
		HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
		table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;

		if (!(entry & HEAP_BLOCK_HAS_NEXT)) {
			break;
		}
	}
}

void *heap_malloc_blocks(struct heap *heap, uint32_t total_blocks)
{
	void *address = 0;

	int start_block = heap_get_start_block(heap, total_blocks);
	if (start_block < 0) {
		goto out;
	}

	address = heap_block_to_address(heap, start_block);

	heap_mark_blocks_taken(heap, start_block, total_blocks);

out:
	return address;
}

void *heap_malloc(struct heap *heap, size_t size)
{
	size_t aligned_size = heap_align_value_to_block_size(size);
	uint32_t total_blocks = aligned_size / HEAP_BLOCK_SIZE;
	return heap_malloc_blocks(heap, total_blocks);
}

void heap_free(struct heap *heap, void *ptr) { heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr)); }
