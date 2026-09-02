#include "gfx_heap.h"

#include "lvgl.h"
#include "lvgl_private.h"
#include "ram_regions.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>
#include <stdbool.h>

#if CONFIG_FREERTOS_SMP
#include "spinlock.h"
#endif

#define GFX_HEAP_STRUCT_SIZE                                              \
    ((sizeof(GfxHeapBlock_t) + (portBYTE_ALIGNMENT - 1))                 \
     & ~(size_t)portBYTE_ALIGNMENT_MASK)

#define GFX_HEAP_MINIMUM_BLOCK_SIZE \
    (GFX_HEAP_STRUCT_SIZE << 1)

#define GFX_HEAP_ALLOCATED_BIT \
    ((size_t)1U << ((sizeof(size_t) * 8U) - 1U))


typedef struct GfxHeapBlock
{
    struct GfxHeapBlock *next;
    size_t size;
} GfxHeapBlock_t;


static GfxHeapBlock_t gfxHeapStart;
static GfxHeapBlock_t *gfxHeapEnd = NULL;

static size_t gfxHeapFreeBytesRemaining = 0;
static size_t gfxHeapMinimumEverFreeBytesRemaining = 0;

static uintptr_t gfxHeapAlignedStart = 0;
static uintptr_t gfxHeapAlignedEnd = 0;

static bool gfxHeapInitialized = false;


#if CONFIG_FREERTOS_SMP

static SPINLOCK_SECTION spinlock_t gfxHeapSpinlock =
    SPIN_LOCK_ACQUIRE_INIT;

#define GfxHeapEnterCritical() \
    vPortEnterCritical(&gfxHeapSpinlock)

#define GfxHeapExitCritical() \
    vPortExitCritical(&gfxHeapSpinlock)

#else

#define GfxHeapEnterCritical() \
    taskENTER_CRITICAL()

#define GfxHeapExitCritical() \
    taskEXIT_CRITICAL()

#endif


static void gfxHeapInitInternal(void);
static void gfxHeapInsertFreeBlock(GfxHeapBlock_t *block);

static void *gfxHeapLvglMalloc(
    size_t size,
    lv_color_format_t colorFormat);

static void gfxHeapLvglFree(void *ptr);


/*
 * Initialize the dedicated PSRAM heap.
 *
 * Initial state:
 *
 * gfxHeapStart
 *      |
 *      v
 * +-----------------------+
 * | one large free block  |
 * +-----------------------+
 *      |
 *      v
 * gfxHeapEnd
 */
static void gfxHeapInitInternal(void)
{
    uintptr_t address;
    size_t totalSize;

    GfxHeapBlock_t *firstFreeBlock;


    address = (uintptr_t)GFX_HEAP_ADDR;
    totalSize = GFX_HEAP_SIZE;

    configASSERT(totalSize >
                 (GFX_HEAP_STRUCT_SIZE * 2));


    /*
     * Align heap start.
     */
    if((address & portBYTE_ALIGNMENT_MASK) != 0)
    {
        uintptr_t alignedAddress;

        alignedAddress =
            (address + portBYTE_ALIGNMENT - 1)
            & ~(uintptr_t)portBYTE_ALIGNMENT_MASK;

        totalSize -= alignedAddress - address;
        address = alignedAddress;
    }

    gfxHeapAlignedStart = address;


    /*
     * Start sentinel.
     */
    gfxHeapStart.size = 0;
    gfxHeapStart.next = (GfxHeapBlock_t *)address;


    /*
     * End sentinel.
     */
    address += totalSize;
    address -= GFX_HEAP_STRUCT_SIZE;

    address &=
        ~(uintptr_t)portBYTE_ALIGNMENT_MASK;

    gfxHeapEnd = (GfxHeapBlock_t *)address;

    gfxHeapEnd->size = 0;
    gfxHeapEnd->next = NULL;

    gfxHeapAlignedEnd = address;


    /*
     * Initially the entire heap is one free block.
     */
    firstFreeBlock =
        (GfxHeapBlock_t *)gfxHeapAlignedStart;

    firstFreeBlock->size =
        gfxHeapAlignedEnd - gfxHeapAlignedStart;

    firstFreeBlock->next =
        gfxHeapEnd;


    gfxHeapStart.next =
        firstFreeBlock;

    gfxHeapFreeBytesRemaining =
        firstFreeBlock->size;

    gfxHeapMinimumEverFreeBytesRemaining =
        firstFreeBlock->size;

    gfxHeapInitialized = true;
}


/*
 * Insert a block back into the free list.
 *
 * The list is kept sorted by address so adjacent blocks
 * can be merged.
 */
static void gfxHeapInsertFreeBlock(GfxHeapBlock_t *block)
{
    GfxHeapBlock_t *iterator;


    iterator = &gfxHeapStart;

    while(iterator->next != gfxHeapEnd &&
          (uintptr_t)iterator->next < (uintptr_t)block)
    {
        iterator = iterator->next;
    }


    /*
     * Merge with previous block if contiguous.
     */
    if(((uint8_t *)iterator + iterator->size) ==
       (uint8_t *)block)
    {
        iterator->size += block->size;
        block = iterator;
    }


    /*
     * Merge with next block if contiguous.
     */
    if(((uint8_t *)block + block->size) ==
       (uint8_t *)iterator->next)
    {
        if(iterator->next != gfxHeapEnd)
        {
            block->size +=
                iterator->next->size;

            block->next =
                iterator->next->next;
        }
        else
        {
            block->next =
                gfxHeapEnd;
        }
    }
    else
    {
        block->next =
            iterator->next;
    }


    if(iterator != block)
    {
        iterator->next = block;
    }
}


void *gfx_heap_malloc(size_t size)
{
    GfxHeapBlock_t *block;
    GfxHeapBlock_t *previousBlock;
    GfxHeapBlock_t *newBlock;

    void *result = NULL;

    size_t wantedSize;


    if(size == 0)
    {
        return NULL;
    }


    /*
     * Check addition overflow.
     */
    if(size >
       SIZE_MAX - GFX_HEAP_STRUCT_SIZE)
    {
        return NULL;
    }

    wantedSize =
        size + GFX_HEAP_STRUCT_SIZE;


    /*
     * Align total block size.
     */
    if((wantedSize &
        portBYTE_ALIGNMENT_MASK) != 0)
    {
        size_t padding;

        padding =
            portBYTE_ALIGNMENT -
            (wantedSize &
             portBYTE_ALIGNMENT_MASK);

        if(wantedSize >
           SIZE_MAX - padding)
        {
            return NULL;
        }

        wantedSize += padding;
    }


    /*
     * Top bit is used as allocation flag.
     */
    if((wantedSize &
        GFX_HEAP_ALLOCATED_BIT) != 0)
    {
        return NULL;
    }


    GfxHeapEnterCritical();


    if(!gfxHeapInitialized)
    {
        gfxHeapInitInternal();
    }


    if(wantedSize >
       gfxHeapFreeBytesRemaining)
    {
        GfxHeapExitCritical();
        return NULL;
    }


    /*
     * First-fit search.
     */
    previousBlock =
        &gfxHeapStart;

    block =
        gfxHeapStart.next;


    while(block != gfxHeapEnd &&
          block->size < wantedSize)
    {
        previousBlock = block;
        block = block->next;
    }


    if(block == gfxHeapEnd)
    {
        GfxHeapExitCritical();
        return NULL;
    }


    /*
     * Remove block from free list.
     */
    previousBlock->next =
        block->next;


    /*
     * Split if enough memory remains.
     */
    if((block->size - wantedSize) >
       GFX_HEAP_MINIMUM_BLOCK_SIZE)
    {
        newBlock =
            (GfxHeapBlock_t *)
            ((uint8_t *)block +
             wantedSize);

        configASSERT(
            ((uintptr_t)newBlock &
             portBYTE_ALIGNMENT_MASK) == 0);


        newBlock->size =
            block->size - wantedSize;

        block->size =
            wantedSize;


        gfxHeapInsertFreeBlock(
            newBlock);
    }


    gfxHeapFreeBytesRemaining -=
        block->size;


    if(gfxHeapFreeBytesRemaining <
       gfxHeapMinimumEverFreeBytesRemaining)
    {
        gfxHeapMinimumEverFreeBytesRemaining =
            gfxHeapFreeBytesRemaining;
    }


    /*
     * Mark allocated.
     */
    block->size |=
        GFX_HEAP_ALLOCATED_BIT;

    block->next = NULL;


    /*
     * Payload begins immediately after block header.
     */
    result =
        (uint8_t *)block +
        GFX_HEAP_STRUCT_SIZE;


    GfxHeapExitCritical();


    configASSERT(
        ((uintptr_t)result &
         portBYTE_ALIGNMENT_MASK) == 0);


    return result;
}


void gfx_heap_free(void *ptr)
{
    GfxHeapBlock_t *block;

    uintptr_t address;
    size_t blockSize;


    if(ptr == NULL)
    {
        return;
    }


    GfxHeapEnterCritical();


    configASSERT(gfxHeapInitialized);


    address =
        (uintptr_t)ptr;


    /*
     * Pointer must belong to this heap.
     */
    configASSERT(
        address >=
        gfxHeapAlignedStart +
        GFX_HEAP_STRUCT_SIZE);

    configASSERT(
        address <
        gfxHeapAlignedEnd);


    block =
        (GfxHeapBlock_t *)
        ((uint8_t *)ptr -
         GFX_HEAP_STRUCT_SIZE);


    /*
     * Must currently be allocated.
     */
    configASSERT(
        (block->size &
         GFX_HEAP_ALLOCATED_BIT) != 0);

    configASSERT(
        block->next == NULL);


    blockSize =
        block->size &
        ~GFX_HEAP_ALLOCATED_BIT;


    /*
     * Mark free.
     */
    block->size =
        blockSize;


    gfxHeapFreeBytesRemaining +=
        blockSize;


    /*
     * Put it back into free list.
     * Adjacent blocks are merged here.
     */
    gfxHeapInsertFreeBlock(
        block);


    GfxHeapExitCritical();
}

void *gfx_heap_malloc_from_back(size_t size)
{
    GfxHeapBlock_t *block;
    GfxHeapBlock_t *previousBlock;

    GfxHeapBlock_t *selectedBlock = NULL;
    GfxHeapBlock_t *selectedPreviousBlock = NULL;
    GfxHeapBlock_t *allocatedBlock;

    void *result = NULL;

    size_t wantedSize;


    if(size == 0)
    {
        return NULL;
    }


    if(size >
       SIZE_MAX - GFX_HEAP_STRUCT_SIZE)
    {
        return NULL;
    }


    wantedSize =
        size + GFX_HEAP_STRUCT_SIZE;


    if((wantedSize &
        portBYTE_ALIGNMENT_MASK) != 0)
    {
        size_t padding;

        padding =
            portBYTE_ALIGNMENT -
            (wantedSize &
             portBYTE_ALIGNMENT_MASK);

        if(wantedSize >
           SIZE_MAX - padding)
        {
            return NULL;
        }

        wantedSize += padding;
    }


    if((wantedSize &
        GFX_HEAP_ALLOCATED_BIT) != 0)
    {
        return NULL;
    }


    GfxHeapEnterCritical();


    if(!gfxHeapInitialized)
    {
        gfxHeapInitInternal();
    }


    if(wantedSize >
       gfxHeapFreeBytesRemaining)
    {
        GfxHeapExitCritical();
        return NULL;
    }


    /*
     * Find the last free block that can satisfy the allocation.
     */
    previousBlock =
        &gfxHeapStart;

    block =
        gfxHeapStart.next;


    while(block != gfxHeapEnd)
    {
        if(block->size >= wantedSize)
        {
            selectedBlock =
                block;

            selectedPreviousBlock =
                previousBlock;
        }

        previousBlock =
            block;

        block =
            block->next;
    }


    if(selectedBlock == NULL)
    {
        GfxHeapExitCritical();
        return NULL;
    }


    /*
     * If the remaining free block would be too small,
     * consume the entire block.
     */
    if((selectedBlock->size - wantedSize) <=
       GFX_HEAP_MINIMUM_BLOCK_SIZE)
    {
        selectedPreviousBlock->next =
            selectedBlock->next;

        allocatedBlock =
            selectedBlock;
    }
    else
    {
        /*
         * Keep the free block at the lower address and
         * carve the allocated block from its high end.
         */
        size_t remainingSize =
            selectedBlock->size -
            wantedSize;

        allocatedBlock =
            (GfxHeapBlock_t *)
            ((uint8_t *)selectedBlock +
             remainingSize);

        configASSERT(
            ((uintptr_t)allocatedBlock &
             portBYTE_ALIGNMENT_MASK) == 0);


        allocatedBlock->size =
            wantedSize;

        selectedBlock->size =
            remainingSize;
    }


    gfxHeapFreeBytesRemaining -=
        allocatedBlock->size;


    if(gfxHeapFreeBytesRemaining <
       gfxHeapMinimumEverFreeBytesRemaining)
    {
        gfxHeapMinimumEverFreeBytesRemaining =
            gfxHeapFreeBytesRemaining;
    }


    allocatedBlock->size |=
        GFX_HEAP_ALLOCATED_BIT;

    allocatedBlock->next =
        NULL;


    result =
        (uint8_t *)allocatedBlock +
        GFX_HEAP_STRUCT_SIZE;


    GfxHeapExitCritical();


    configASSERT(
        ((uintptr_t)result &
         portBYTE_ALIGNMENT_MASK) == 0);


    return result;
}

#include <string.h>

void *gfx_heap_realloc(void *ptr, size_t size)
{
    GfxHeapBlock_t *block;
    size_t blockSize;
    size_t oldSize;
    size_t copySize;
    void *newPtr;

    if(ptr == NULL)
    {
        return gfx_heap_malloc(size);
    }

    if(size == 0)
    {
        gfx_heap_free(ptr);
        return NULL;
    }

    block = (GfxHeapBlock_t *)
        ((uint8_t *)ptr - GFX_HEAP_STRUCT_SIZE);

    configASSERT(
        (block->size & GFX_HEAP_ALLOCATED_BIT) != 0);

    blockSize =
        block->size & ~GFX_HEAP_ALLOCATED_BIT;

    oldSize =
        blockSize - GFX_HEAP_STRUCT_SIZE;

    /*
     * 이미 현재 block 안에 들어가면 그대로 사용.
     * shrink 시 block을 다시 쪼개지는 않음.
     */
    if(size <= oldSize)
    {
        return ptr;
    }

    newPtr = gfx_heap_malloc(size);

    if(newPtr == NULL)
    {
        return NULL;
    }

    copySize =
        oldSize < size ? oldSize : size;

    memcpy(newPtr, ptr, copySize);

    gfx_heap_free(ptr);

    return newPtr;
}

/*
 * LVGL image draw-buffer callback.
 *
 * LVGL's default allocator also reserves
 * LV_DRAW_BUF_ALIGN - 1 bytes because the returned
 * pointer is aligned afterward.
 */
static void *gfxHeapLvglMalloc(size_t size, lv_color_format_t colorFormat)
{
    LV_UNUSED(colorFormat);


    if(size >
       SIZE_MAX - (LV_DRAW_BUF_ALIGN - 1))
    {
        return NULL;
    }


    return gfx_heap_malloc(
        size +
        LV_DRAW_BUF_ALIGN -
        1);
}


static void gfxHeapLvglFree(void *ptr)
{
    gfx_heap_free(ptr);
}


void gfx_heap_init(void)
{
    lv_draw_buf_handlers_t *handlers;


    /*
     * Must be called after lv_init().
     */
    configASSERT(
        lv_is_initialized());


    GfxHeapEnterCritical();

    if(!gfxHeapInitialized)
    {
        gfxHeapInitInternal();
    }

    GfxHeapExitCritical();


    /*
     * Replace ONLY the backing-buffer malloc/free
     * callbacks for LVGL image draw buffers.
     *
     * copy / align / stride handlers remain LVGL defaults.
     */
    handlers =
        lv_draw_buf_get_image_handlers();

    configASSERT(
        handlers != NULL);


    handlers->buf_malloc_cb =
        gfxHeapLvglMalloc;

    handlers->buf_free_cb =
        gfxHeapLvglFree;
}


size_t gfx_heap_get_free_size(void)
{
    size_t freeSize;


    GfxHeapEnterCritical();

    freeSize =
        gfxHeapFreeBytesRemaining;

    GfxHeapExitCritical();


    return freeSize;
}


size_t gfx_heap_get_largest_free_block(void)
{
    GfxHeapBlock_t *block;

    size_t largestBlock = 0;


    GfxHeapEnterCritical();


    if(gfxHeapInitialized)
    {
        block =
            gfxHeapStart.next;


        while(block != NULL &&
              block != gfxHeapEnd)
        {
            if(block->size >
               largestBlock)
            {
                largestBlock =
                    block->size;
            }

            block =
                block->next;
        }
    }


    GfxHeapExitCritical();


    /*
     * The free block includes the allocation header.
     * Return approximate usable payload size.
     */
    if(largestBlock >
       GFX_HEAP_STRUCT_SIZE)
    {
        largestBlock -=
            GFX_HEAP_STRUCT_SIZE;
    }
    else
    {
        largestBlock = 0;
    }


    return largestBlock;
}

#ifndef LODEPNG_COMPILE_ALLOCATORS
void *lodepng_malloc(size_t size)
{
    return gfx_heap_malloc_from_back(size);
}

void *lodepng_realloc(void *ptr, size_t size)
{
    return gfx_heap_realloc(ptr, size);
}

void lodepng_free(void *ptr)
{
    gfx_heap_free(ptr);
}
#endif /* !LODEPNG_COMPILE_ALLOCATORS == CUSTOM ALLOCATOR */ 