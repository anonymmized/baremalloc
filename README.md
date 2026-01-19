# baremalloc

baremalloc is a minimal educational memory allocator (a tiny malloc/free clone) built on top of a fixed-size static heap buffer.
It is designed to demonstrate the core ideas behind dynamic memory allocation:
	•	block headers (metadata)
	•	first-fit search
	•	block splitting (split)
	•	adjacent free block merging (coalesce)
	•	alignment handling

This is not meant to replace the system allocator.



## Features
	•	Custom API:
	•	malloc_my(size_t size)
	•	free_my(void* ptr)
	•	Fixed heap backed by a static byte array:
	•	static uint8_t heap[HEAP_SIZE]
	•	8-byte alignment via ALIGN_UP()
	•	First-fit allocation strategy
	•	Block splitting when a free block is larger than requested
	•	Coalescing (merging) of adjacent free blocks on free_my()
	•	dump_heap() helper to inspect heap state



## Project Structure

Single file allocator + test:
	•	baremalloc.c (contains allocator + main() test)



## Heap Layout

The allocator manages a fixed region:
```C
static uint8_t heap[HEAP_SIZE];
```
Each allocation is stored like this in memory:
```C
[ block_header ][ payload (user data) ]
```
## Block header
```C
typedef struct block_header {
    size_t size;                 // payload size in bytes
    int free;                    // 1 = free, 0 = used
    struct block_header* next;   // next block in the list
} block_header;
```
## Header ↔ payload conversion
The allocator returns a pointer to the payload, not the header.
```C
static inline void* header_to_payload(block_header* h) {
    return (void*)(h + 1);
}

static inline block_header* payload_to_header(void* p) {
    return ((block_header*)p) - 1;
}
```



## How it works
### 1) Heap initialization
```C
void heap_init();
```
On first use, heap_init() builds a single free block covering the entire heap:
	•	heap_start points to the first block header in heap
	•	heap_start->size = HEAP_SIZE - sizeof(block_header)
	•	heap_start->free = 1

### 2) Finding a free block (first-fit)
```C
block_header* find_fit(size_t size);
```
The allocator walks the block list and returns the first free block where:
	•	block->free == 1
	•	block->size >= requested_size

### 3) Splitting a block
```C
void split_block(block_header* b, size_t size);
```
If the chosen free block is significantly larger than needed, it is split into:

	•	a used block of size bytes
	•	a new free block containing the remaining bytes

Splitting is skipped if the leftover space is too small to form a useful new block.
### 4) Coalescing free blocks
```C
void coalecse();
```
After freeing a block, the allocator merges adjacent free blocks:
```C
[FREE A][FREE B]  ->  [FREE (A+B)]
```
This reduces fragmentation over time.

## API

**void\* malloc_my(size_t size)**

	•	Returns a pointer to a payload area of at least size bytes
	•	The requested size is rounded up to an 8-byte boundary
	•	Returns NULL if no block fits

Example:
```C
char* p = malloc_my(100);
if (!p) {
    printf("Out of memory\n");
}
```
**void free_my(void\* ptr)**

	•	Marks the block as free
	•	Runs coalescing to merge adjacent free blocks
	•	free_my(NULL) does nothing

Example:
```C
free_my(p);
```
**void dump_heap()**

Prints a readable heap map of all blocks:

	•	header address
	•	payload size
	•	free flag
	•	pointer to next block

Example:
```C
dump_heap();
```
## Build & Run

### GCC (Linux/macOS)
```bash
$ gcc -O0 -g baremalloc.c -o baremalloc
$ ./baremalloc
```
### Clang (Linux/macOS)
```bash
$ clang -O0 -g baremalloc.c -o baremalloc
$ ./baremalloc
```
## Example Flow

The included main() demonstrates:

	1.	initialize heap
	2.	allocate a small block (a)
	3.	allocate a larger block (b)
	4.	free a
	5.	allocate another small block (c)
	6.	free b
	7.	free c
	8.	allocate a large block
	9.	free it again

During this flow you can observe:

	•	how blocks split when allocating
	•	how free blocks merge again after free_my()
