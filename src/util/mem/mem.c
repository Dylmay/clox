#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "mem.h"

#ifdef DEBUG_LOG_GC
#define log_debug(args...) (printf(args))
#else
#define log_debug(args...) (true)
#endif

typedef uint8_t memory_flags_t;

#pragma region memory_meta

struct __attribute__((packed)) mem_metadata {
	memory_flags_t flags;
};

#define METADATA_NO_FLAGS (0)
#define METADATA_MARKED (1 << 0)

bool mem_is_marked(struct mem_metadata *meta)
{
	return meta->flags & METADATA_MARKED;
}

#pragma endregion

void _collect_garbage();
void _mark_roots();

void *reallocate(void *pointer, size_t old_sz, size_t new_sz)
{
	struct mem_metadata *metadata = NULL;

	if (pointer != NULL) {
		log_debug("fetching metadata\n");
		metadata = pointer - sizeof(struct mem_metadata);
	}
	/*
	we need:
	1. some metadata to allow marking, etc.
	2. to understand when the VM is done with an object
	3. a list of all objects, with the ability to track if they're reachable
	4. the above is why it makes sense for it to live within the VM
	5. a generic GC is likely not worth the effort
	 */

	void *offset_pointer =
		pointer - (metadata != NULL ? sizeof(struct mem_metadata) : 0);

	if (new_sz > old_sz) {
#ifdef DEBUG_STRESS_GC
		log_debug("Rlox Memory: Reallocating %lu -> %lu\n", old_sz,
			  new_sz);
		_collect_garbage();
#endif
	}

	if (new_sz == 0) {
		log_debug("Rlox Memory: Freeing %lu -> %lu\n", old_sz, new_sz);

		assert((pointer, "Pointer does not exist"));

		free(offset_pointer);

		return NULL;
	}

	void *result =
		realloc(offset_pointer, new_sz + sizeof(struct mem_metadata));

	if (result == NULL) {
		printf("Rlox Memory Error: Unable to allocate additional memory (Prev size %lu; New size %lu)\n",
		       old_sz, new_sz);
		exit(100);
	}

	*(struct mem_metadata *)result =
		(struct mem_metadata){ .flags = METADATA_NO_FLAGS };

	log_debug("done\n");

	return (result + sizeof(struct mem_metadata));
}

void _collect_garbage()
{
	log_debug("GC: begin\n");

	_mark_roots();

	log_debug("GC: end\n");
}

void _mark_roots()
{
}