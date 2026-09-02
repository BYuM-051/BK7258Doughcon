#ifndef GFX_HEAP_H
#define GFX_HEAP_H

#define GFX_HEAP_ADDR      ((uint8_t *)CONFIG_AP_PSRAM_GRAPHICS_ADDR)
#define GFX_HEAP_SIZE      ((size_t)CONFIG_AP_PSRAM_GRAPHICS_SIZE)
#define GFX_HEAP_END        (GFX_HEAP_ADDR + GFX_HEAP_SIZE)

#include <stddef.h>

void gfx_heap_init(void);

void* gfx_heap_malloc(size_t size);
void gfx_heap_free(void *ptr);
void* gfx_heap_realloc(void *ptr, size_t size);
void* gfx_heap_malloc_from_back(size_t size);

size_t gfx_heap_get_free_size(void);
size_t gfx_heap_get_largest_free_block(void);

#endif /* GFX_HEAP_H */