/**
 * @file lv_mem.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_mem_private.h"
#include "lv_string.h"
#include "../misc/lv_assert.h"
#include "../misc/lv_log.h"
#include "../core/lv_global.h"

#if LV_USE_OS == LV_OS_PTHREAD
    #include <pthread.h>
#endif

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM
    #include <os/mem.h>
#endif
void *pvPortRealloc( void *pv, size_t size );

/* PSRAM 단편화로 malloc/realloc이 실패하면(NULL 반환), 공유 이미지 캐시를
 * 비우고 한 번 더 시도한다 — 순환 include를 피하기 위한 최소 전방 선언.
 * heap_4.c의 PSRAM_MALLOC_ASSERT_ON_FAIL=0일 때만 의미 있음: 그 전엔
 * psram_malloc_cm이 실패 시 NULL을 반환하기 전에 BK_ASSERT(0)로 즉시
 * 하드크래시했으므로 이 재시도 코드에 도달할 수 없었다. */
extern void lv_image_cache_drop(const void * src);
/*********************
 *      DEFINES
 *********************/
/*memset the allocated memories to 0xaa and freed memories to 0xbb (just for testing purposes)*/
#ifndef LV_MEM_ADD_JUNK
    #define LV_MEM_ADD_JUNK  0
#endif

#define zero_mem LV_GLOBAL_DEFAULT()->memory_zero

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  GLOBAL PROTOTYPES
 **********************/
void * lv_malloc_core(size_t size);
void * lv_realloc_core(void * p, size_t new_size);
void lv_free_core(void * p);
void lv_mem_monitor_core(lv_mem_monitor_t * mon_p);
lv_result_t lv_mem_test_core(void);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/
#if LV_USE_LOG && LV_LOG_TRACE_MEM
    #define LV_TRACE_MEM(...) LV_LOG_TRACE(__VA_ARGS__)
#else
    #define LV_TRACE_MEM(...)
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/* lv_malloc/lv_malloc_zeroed 공통 raw alloc — 원본 매크로 분기를 그대로 유지한
 * 헬퍼로 뽑아, 실패 시 캐시 드롭 후 재시도할 때도 같은 분기를 재사용한다. */
static void * _lv_mem_raw_alloc(size_t size)
{
#if (LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM)
#if CONFIG_PSRAM_AS_SYS_MEMORY
    return psram_malloc(size);
#else
    return os_malloc(size);
#endif
#else
    return lv_malloc_core(size);
#endif
}

void * lv_malloc(size_t size)
{
    LV_TRACE_MEM("allocating %lu bytes", (unsigned long)size);
    if(size == 0) {
        LV_TRACE_MEM("using zero_mem");
        return &zero_mem;
    }

    void * alloc = _lv_mem_raw_alloc(size);

    if(alloc == NULL) {
        /* 단편화로 인한 실패일 수 있음 — 공유 이미지 캐시를 통째로 비워
         * 인접한 빈 블록이 병합될 기회를 준 뒤 한 번 더 시도.
         * (psram_malloc_cm의 PSRAM_MALLOC_ASSERT_ON_FAIL=0일 때만 여기 도달함 —
         * 1이면 실패 시 NULL 반환 전에 이미 하드크래시했음.) */
        LV_LOG_INFO("malloc failed (%lu bytes) — dropping image cache and retrying", (unsigned long)size);
        lv_image_cache_drop(NULL);
        alloc = _lv_mem_raw_alloc(size);
    }

    if(alloc == NULL) {
        LV_LOG_INFO("couldn't allocate memory (%lu bytes)", (unsigned long)size);
#if LV_LOG_LEVEL <= LV_LOG_LEVEL_INFO
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        LV_LOG_INFO("used: %zu (%3d %%), frag: %3d %%, biggest free: %zu",
                    mon.total_size - mon.free_size, mon.used_pct, mon.frag_pct,
                    mon.free_biggest_size);
#endif
        return NULL;
    }

#if LV_MEM_ADD_JUNK
    lv_memset(alloc, 0xaa, size);
#endif

    LV_TRACE_MEM("allocated at %p", alloc);
    return alloc;
}

void * lv_malloc_zeroed(size_t size)
{
    LV_TRACE_MEM("allocating %lu bytes", (unsigned long)size);
    if(size == 0) {
        LV_TRACE_MEM("using zero_mem");
        return &zero_mem;
    }

    void * alloc = _lv_mem_raw_alloc(size);
    if(alloc == NULL) {
        LV_LOG_INFO("malloc_zeroed failed (%lu bytes) — dropping image cache and retrying", (unsigned long)size);
        lv_image_cache_drop(NULL);
        alloc = _lv_mem_raw_alloc(size);
    }
    if(alloc == NULL) {
        LV_LOG_INFO("couldn't allocate memory (%lu bytes)", (unsigned long)size);
#if LV_LOG_LEVEL <= LV_LOG_LEVEL_INFO
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        LV_LOG_INFO("used: %zu (%3d %%), frag: %3d %%, biggest free: %zu",
                    mon.total_size - mon.free_size, mon.used_pct, mon.frag_pct,
                    mon.free_biggest_size);
#endif
        return NULL;
    }

    lv_memzero(alloc, size);

    LV_TRACE_MEM("allocated at %p", alloc);
    return alloc;
}

void * lv_calloc(size_t num, size_t size)
{
    LV_TRACE_MEM("allocating number of %zu each %zu bytes", num, size);
    return lv_malloc_zeroed(num * size);
}

void * lv_zalloc(size_t size)
{
    return lv_malloc_zeroed(size);
}

void lv_free(void * data)
{
    LV_TRACE_MEM("freeing %p", data);
    if(data == &zero_mem) return;
    if(data == NULL) return;

#if (LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM)
    os_free(data);
#else
    lv_free_core(data);
#endif
}

void * lv_reallocf(void * data_p, size_t new_size)
{
    void * new = lv_realloc(data_p, new_size);
    if(!new) {
        lv_free(data_p);
    }
    return new;
}

static void * _lv_mem_raw_realloc(void * data_p, size_t new_size)
{
#if (LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM)
#if CONFIG_PSRAM_AS_SYS_MEMORY
    return psram_realloc(data_p, new_size);
#else
    return os_realloc(data_p, new_size);
#endif
#else
    return lv_realloc_core(data_p, new_size);
#endif
}

void * lv_realloc(void * data_p, size_t new_size)
{
    LV_TRACE_MEM("reallocating %p with %lu size", data_p, (unsigned long)new_size);
    if(new_size == 0) {
        LV_TRACE_MEM("using zero_mem");
        lv_free(data_p);
        return &zero_mem;
    }

    if(data_p == &zero_mem) return lv_malloc(new_size);

    void * new_p = _lv_mem_raw_realloc(data_p, new_size);

    if(new_p == NULL) {
        /* realloc 실패 시 원본 data_p는 표준 realloc 규약상 그대로 유효하게
         * 남아있음 — 캐시를 비워 단편화를 완화한 뒤 같은 인자로 한 번 더 시도. */
        LV_LOG_INFO("realloc failed (%lu bytes) — dropping image cache and retrying", (unsigned long)new_size);
        lv_image_cache_drop(NULL);
        new_p = _lv_mem_raw_realloc(data_p, new_size);
    }

    if(new_p == NULL) {
        LV_LOG_ERROR("couldn't reallocate memory");
        return NULL;
    }

    LV_TRACE_MEM("reallocated at %p", new_p);
    return new_p;
}

void * lv_psram_malloc(size_t size)
{
    LV_TRACE_MEM("allocating %lu bytes", (unsigned long)size);
    if(size == 0) {
        LV_TRACE_MEM("using zero_mem");
        return &zero_mem;
    }

    void * alloc = psram_malloc(size);
    if(alloc == NULL) {
        LV_LOG_INFO("psram_malloc failed (%lu bytes) — dropping image cache and retrying", (unsigned long)size);
        lv_image_cache_drop(NULL);
        alloc = psram_malloc(size);
    }
    if(alloc == NULL) {
        LV_LOG_INFO("couldn't allocate memory (%lu bytes)", (unsigned long)size);
#if LV_LOG_LEVEL <= LV_LOG_LEVEL_INFO
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        LV_LOG_INFO("used: %zu (%3d %%), frag: %3d %%, biggest free: %zu",
                    mon.total_size - mon.free_size, mon.used_pct, mon.frag_pct,
                    mon.free_biggest_size);
#endif
        return NULL;
    }

#if LV_MEM_ADD_JUNK
    lv_memset(alloc, 0xaa, size);
#endif

    LV_TRACE_MEM("allocated at %p", alloc);
    return alloc;
}

void * lv_psram_realloc(void * data_p, size_t new_size)
{
    LV_TRACE_MEM("reallocating %p with %lu size", data_p, (unsigned long)new_size);
    if(new_size == 0) {
        LV_TRACE_MEM("using zero_mem");
        lv_free(data_p);
        return &zero_mem;
    }

    if(data_p == &zero_mem) return lv_psram_malloc(new_size);

    void * new_p = psram_realloc(data_p, new_size);
    if(new_p == NULL) {
        LV_LOG_INFO("psram_realloc failed (%lu bytes) — dropping image cache and retrying", (unsigned long)new_size);
        lv_image_cache_drop(NULL);
        new_p = psram_realloc(data_p, new_size);
    }
    if(new_p == NULL) {
        LV_LOG_ERROR("couldn't reallocate memory");
        return NULL;
    }

    LV_TRACE_MEM("reallocated at %p", new_p);
    return new_p;
}

lv_result_t lv_mem_test(void)
{
    if(zero_mem != ZERO_MEM_SENTINEL) {
        LV_LOG_WARN("zero_mem is written");
        return LV_RESULT_INVALID;
    }

#if (LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM)
    return LV_RESULT_OK;
#else
    return lv_mem_test_core();
#endif
}

void lv_mem_monitor(lv_mem_monitor_t * mon_p)
{
#if (LV_USE_STDLIB_MALLOC != LV_STDLIB_CUSTOM)
    lv_memzero(mon_p, sizeof(lv_mem_monitor_t));
    lv_mem_monitor_core(mon_p);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
