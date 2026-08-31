/**
 * @file lv_lodepng.c
 *
 */
#include "../../../../../../../projects/lvgl/korea_test/ap/ui/ui_config.h"

#if UI_LODEPNG_565A8
/**
 * @file lv_lodepng.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../draw/lv_image_decoder_private.h"
#include "../../../lvgl.h"
#include "../../core/lv_global.h"
#if LV_USE_LODEPNG

#include "lv_lodepng.h"
#include "lodepng.h"
#include <stdlib.h>
#include <stdio.h>
#define TAG "[lv_lodepng.c] "
#define bk_printf(fmt, ...) do {if(0) printf(fmt, ##__VA_ARGS__); } while(0) // disable printf

/*********************
 *      DEFINES
 *********************/

#define DECODER_NAME    "LODEPNG"

#define image_cache_draw_buf_handlers &(LV_GLOBAL_DEFAULT()->image_cache_draw_buf_handlers)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * src, lv_image_header_t * header);
static lv_result_t decoder_open(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc);
static void decoder_close(lv_image_decoder_t * dec, lv_image_decoder_dsc_t * dsc);
static lv_draw_buf_t * decode_png_data(const void * png_data, size_t png_data_size);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Register the PNG decoder functions in LVGL
 */
void lv_lodepng_init(void)
{
    lv_image_decoder_t * dec = lv_image_decoder_create();
    lv_image_decoder_set_info_cb(dec, decoder_info);
    lv_image_decoder_set_open_cb(dec, decoder_open);
    lv_image_decoder_set_close_cb(dec, decoder_close);

    dec->name = DECODER_NAME;
}

void lv_lodepng_deinit(void)
{
    lv_image_decoder_t * dec = NULL;
    while((dec = lv_image_decoder_get_next(dec)) != NULL)
    {
        if(dec->info_cb == decoder_info)
        {
            lv_image_decoder_delete(dec);
            break;
        }
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Get info about a PNG image
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       image descriptor containing the source and type of the image and other info.
 * @param header    image information is set in header parameter
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't get the info
 */
static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc, lv_image_header_t * header)
{
    LV_UNUSED(decoder); /*Unused*/

    lv_image_src_t src_type = dsc->src_type;          /*Get the source type*/

    if(src_type == LV_IMAGE_SRC_FILE || src_type == LV_IMAGE_SRC_VARIABLE)
    {
        uint32_t * size;
        static const uint8_t magic[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
        uint8_t buf[24];

        /*If it's a PNG file...*/
        if(src_type == LV_IMAGE_SRC_FILE)
        {
            /* Read the width and height from the file. They have a constant location:
             * [16..19]&#58; width
             * [20..23]&#58; height
             */
            uint32_t rn;
            lv_fs_read(&dsc->file, buf, sizeof(buf), &rn);

            if(rn != sizeof(buf))
            {
                return LV_RESULT_INVALID;
            }

            if(lv_memcmp(buf, magic, sizeof(magic)) != 0)
            {
                return LV_RESULT_INVALID;
            }

            size = (uint32_t *)&buf[16];
        }
        /*If it's a PNG file in a C array...*/
        else
        {
            const lv_image_dsc_t * img_dsc = dsc->src;
            const uint32_t data_size = img_dsc->data_size;
            size = ((uint32_t *)img_dsc->data) + 4;

            if(data_size < sizeof(magic))
            {
                return LV_RESULT_INVALID;
            }

            if(lv_memcmp(img_dsc->data, magic, sizeof(magic)) != 0)
            {
                return LV_RESULT_INVALID;
            }
        }

        /*Save the data in the header*/
        header->cf = LV_COLOR_FORMAT_RGB565A8;

        /*The width and height are stored in Big endian format so convert them to little endian*/
        header->w = (int32_t)((size[0] & 0xff000000) >> 24) +
                    ((size[0] & 0x00ff0000) >> 8);

        header->h = (int32_t)((size[1] & 0xff000000) >> 24) +
                    ((size[1] & 0x00ff0000) >> 8);

        return LV_RESULT_OK;
    }

    return LV_RESULT_INVALID;         /*If didn't succeeded earlier then it's an error*/
}

/**
 * Open a PNG image and decode it into dsc.decoded
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       decoded image descriptor
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't open the image
 */
static lv_result_t decoder_open(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);
    LV_PROFILER_DECODER_BEGIN_TAG("lv_lodepng_decoder_open");

    const uint8_t * png_data = NULL;
    size_t png_data_size = 0;

    if(dsc->src_type == LV_IMAGE_SRC_FILE)
    {
        const char * fn = dsc->src;

        /*Load the file*/
        unsigned error = lodepng_load_file((void *)&png_data, &png_data_size, fn);

        if(error)
        {
            if(png_data != NULL)
            {
                lv_free((void *)png_data);
            }

            LV_LOG_WARN("error %u: %s\n", error, lodepng_error_text(error));
            LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
            return LV_RESULT_INVALID;
        }
    }
    else if(dsc->src_type == LV_IMAGE_SRC_VARIABLE)
    {
        const lv_image_dsc_t * img_dsc = dsc->src;
        png_data = img_dsc->data;
        png_data_size = img_dsc->data_size;
    }
    else
    {
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t * decoded = decode_png_data(png_data, png_data_size);

    if(dsc->src_type == LV_IMAGE_SRC_FILE)
    {
        lv_free((void *)png_data);
    }

    if(!decoded)
    {
        LV_LOG_WARN("Error decoding PNG");
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t * adjusted = lv_image_decoder_post_process(dsc, decoded);

    if(adjusted == NULL)
    {
        lv_draw_buf_destroy(decoded);
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_INVALID;
    }

    /*The adjusted draw buffer is newly allocated.*/
    if(adjusted != decoded)
    {
        lv_draw_buf_destroy(decoded);
        decoded = adjusted;
    }

    dsc->decoded = decoded;

    if(dsc->args.no_cache)
    {
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_OK;
    }

    /*If the image cache is disabled, just return the decoded image*/
    if(!lv_image_cache_is_enabled())
    {
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_OK;
    }

    /*Add the decoded image to the cache*/
    lv_image_cache_data_t search_key;
    search_key.src_type = dsc->src_type;
    search_key.src = dsc->src;
    search_key.slot.size = decoded->data_size;

    lv_cache_entry_t * entry =
        lv_image_decoder_add_to_cache(decoder, &search_key, decoded, NULL);

    if(entry == NULL)
    {
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_INVALID;
    }

    dsc->cache_entry = entry;

    LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
    return LV_RESULT_OK;
}

/**
 * Close PNG image and free data
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       decoded image descriptor
 */
static void decoder_close(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);

    if(dsc->args.no_cache ||
       !lv_image_cache_is_enabled())
    {
        lv_draw_buf_destroy((lv_draw_buf_t *)dsc->decoded);
    }
}

/**
 * Decode PNG as RGBA8888 temporarily and convert it to RGB565A8.
 *
 * RGB565A8 buffer layout used by LVGL:
 *
 * [ RGB565 RGB565 RGB565 ... ]
 * [   A8     A8     A8   ... ]
 *
 * The RGB565 plane uses decoded->header.stride bytes per row.
 * The A8 plane begins immediately after the RGB565 plane and
 * uses decoded->header.stride / 2 bytes per row.
 */
static lv_draw_buf_t * decode_png_data(const void * png_data, size_t png_data_size)
{
    unsigned png_width;
    unsigned png_height;

    lv_draw_buf_t * decoded32 = NULL;

    uint32_t totalStart = lv_tick_get();
    uint32_t decodeStart = lv_tick_get();

    /*
     * The modified LodePNG implementation in this LVGL port returns
     * an lv_draw_buf_t containing RGBA8888 pixel data.
     */
    unsigned error = lodepng_decode32(
        (unsigned char **)&decoded32,
        &png_width,
        &png_height,
        png_data,
        png_data_size
    );

    uint32_t decodeMs = lv_tick_elaps(decodeStart);

    if(error)
    {
        if(decoded32 != NULL)
        {
            lv_draw_buf_destroy(decoded32);
        }

        return NULL;
    }

    /*
     * Measure the complete extra RGB565A8 stage separately from decode32:
     * allocation + clear + conversion loop + temporary ARGB8888 release.
     */
    uint32_t stage565Start = lv_tick_get();

    /*
     * Allocate the final image-cache buffer as RGB565A8.
     *
     * LVGL allocates:
     *
     *     RGB565 stride * height
     *     +
     *     (RGB565 stride / 2) * height
     *
     * for the A8 plane.
     */
    lv_draw_buf_t * decoded = lv_draw_buf_create_ex(
        image_cache_draw_buf_handlers,
        png_width,
        png_height,
        LV_COLOR_FORMAT_RGB565A8,
        0
    );

    if(decoded == NULL)
    {
        lv_draw_buf_destroy(decoded32);
        return NULL;
    }

    /*
     * Clear padding bytes as stride can be larger than width * 2
     * because of LV_DRAW_BUF_STRIDE_ALIGN.
     */
    lv_memzero(decoded->data, decoded->data_size);

    const uint8_t * srcData = decoded32->data;

    const uint32_t srcStride = decoded32->header.stride;
    const uint32_t dstStride = decoded->header.stride;

    uint8_t * colorData = decoded->data;

    /*
     * RGB565A8's alpha plane starts after the complete RGB565 plane.
     */
    uint8_t * alphaData =
        decoded->data +
        (dstStride * png_height);

    const uint32_t alphaStride = dstStride / 2;

    /* Pure pixel conversion loop timing. */
    uint32_t convertStart = lv_tick_get();

    for(uint32_t y = 0; y < png_height; y++)
    {
        const uint8_t * srcRow =
            srcData +
            (y * srcStride);

        uint16_t * colorRow =
            (uint16_t *)(colorData +
                         (y * dstStride));

        uint8_t * alphaRow =
            alphaData +
            (y * alphaStride);

        for(uint32_t x = 0; x < png_width; x++)
        {
            /*
             * lodepng_decode32() output:
             *
             * byte 0 = R
             * byte 1 = G
             * byte 2 = B
             * byte 3 = A
             */
            const uint8_t r = srcRow[x * 4 + 0];
            const uint8_t g = srcRow[x * 4 + 1];
            const uint8_t b = srcRow[x * 4 + 2];
            const uint8_t a = srcRow[x * 4 + 3];

            /*
             * RGB888 -> RGB565
             *
             * RRRRR GGGGGG BBBBB
             */
            colorRow[x] =
                ((uint16_t)(r & 0xF8) << 8) |
                ((uint16_t)(g & 0xFC) << 3) |
                ((uint16_t)(b & 0xF8) >> 3);

            alphaRow[x] = a;
        }
    }

    uint32_t convertMs = lv_tick_elaps(convertStart);

    /*
     * ARGB8888/RGBA8888 temporary decode buffer is no longer needed.
     * Only RGB565A8 remains resident and can enter the LVGL image cache.
     */
    lv_draw_buf_destroy(decoded32);

    uint32_t stage565Ms = lv_tick_elaps(stage565Start);
    uint32_t totalMs = lv_tick_elaps(totalStart);


    bk_printf(TAG "[PNG_PERF] mode=RGB565A8 size=%ux%u decode32=%lu ms convert_loop=%lu ms 565A8_stage=%lu ms total=%lu ms\\n",
        png_width,
        png_height,
        (unsigned long)decodeMs,
        (unsigned long)convertMs,
        (unsigned long)stage565Ms,
        (unsigned long)totalMs);

    return decoded;
}

#endif /*LV_USE_LODEPNG*/
#else

/*********************
 *      INCLUDES
 *********************/
#include "../../draw/lv_image_decoder_private.h"
#include "../../../lvgl.h"
#include "../../core/lv_global.h"
#if LV_USE_LODEPNG

#include "lv_lodepng.h"
#include "lodepng.h"
#include <stdlib.h>
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/

#define DECODER_NAME    "LODEPNG"

#define image_cache_draw_buf_handlers &(LV_GLOBAL_DEFAULT()->image_cache_draw_buf_handlers)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * src, lv_image_header_t * header);
static lv_result_t decoder_open(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc);
static void decoder_close(lv_image_decoder_t * dec, lv_image_decoder_dsc_t * dsc);
static void convert_color_depth(uint8_t * img_p, uint32_t px_cnt);
static lv_draw_buf_t * decode_png_data(const void * png_data, size_t png_data_size);
/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Register the PNG decoder functions in LVGL
 */
void lv_lodepng_init(void)
{
    lv_image_decoder_t * dec = lv_image_decoder_create();
    lv_image_decoder_set_info_cb(dec, decoder_info);
    lv_image_decoder_set_open_cb(dec, decoder_open);
    lv_image_decoder_set_close_cb(dec, decoder_close);

    dec->name = DECODER_NAME;
}

void lv_lodepng_deinit(void)
{
    lv_image_decoder_t * dec = NULL;
    while((dec = lv_image_decoder_get_next(dec)) != NULL) {
        if(dec->info_cb == decoder_info) {
            lv_image_decoder_delete(dec);
            break;
        }
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Get info about a PNG image
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       image descriptor containing the source and type of the image and other info.
 * @param header    image information is set in header parameter
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't get the info
 */
static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc, lv_image_header_t * header)
{
    LV_UNUSED(decoder); /*Unused*/

    lv_image_src_t src_type = dsc->src_type;          /*Get the source type*/

    if(src_type == LV_IMAGE_SRC_FILE || src_type == LV_IMAGE_SRC_VARIABLE) {
        uint32_t * size;
        static const uint8_t magic[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
        uint8_t buf[24];

        /*If it's a PNG file...*/
        if(src_type == LV_IMAGE_SRC_FILE) {
            /* Read the width and height from the file. They have a constant location:
            * [16..19]: width
            * [20..23]: height
            */
            uint32_t rn;
            lv_fs_read(&dsc->file, buf, sizeof(buf), &rn);

            if(rn != sizeof(buf)) return LV_RESULT_INVALID;

            if(lv_memcmp(buf, magic, sizeof(magic)) != 0) return LV_RESULT_INVALID;

            size = (uint32_t *)&buf[16];
        }
        /*If it's a PNG file in a  C array...*/
        else {
            const lv_image_dsc_t * img_dsc = dsc->src;
            const uint32_t data_size = img_dsc->data_size;
            size = ((uint32_t *)img_dsc->data) + 4;

            if(data_size < sizeof(magic)) return LV_RESULT_INVALID;
            if(lv_memcmp(img_dsc->data, magic, sizeof(magic)) != 0) return LV_RESULT_INVALID;
        }

        /*Save the data in the header*/
        header->cf = LV_COLOR_FORMAT_ARGB8888;
        /*The width and height are stored in Big endian format so convert them to little endian*/
        header->w = (int32_t)((size[0] & 0xff000000) >> 24) + ((size[0] & 0x00ff0000) >> 8);
        header->h = (int32_t)((size[1] & 0xff000000) >> 24) + ((size[1] & 0x00ff0000) >> 8);

        return LV_RESULT_OK;
    }

    return LV_RESULT_INVALID;         /*If didn't succeeded earlier then it's an error*/
}

/**
 * Open a PNG image and decode it into dsc.decoded
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       decoded image descriptor
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't open the image
 */
static lv_result_t decoder_open(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);
    LV_PROFILER_DECODER_BEGIN_TAG("lv_lodepng_decoder_open");

    const uint8_t * png_data = NULL;
    size_t png_data_size = 0;
    if(dsc->src_type == LV_IMAGE_SRC_FILE) {
        const char * fn = dsc->src;

        /*Load the file*/
        unsigned error = lodepng_load_file((void *)&png_data, &png_data_size, fn);
        if(error) {
            if(png_data != NULL) {
                lv_free((void *)png_data);
            }
            LV_LOG_WARN("error %u: %s\n", error, lodepng_error_text(error));
            LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
            return LV_RESULT_INVALID;
        }
    }
    else if(dsc->src_type == LV_IMAGE_SRC_VARIABLE) {
        const lv_image_dsc_t * img_dsc = dsc->src;
        png_data = img_dsc->data;
        png_data_size = img_dsc->data_size;
    }
    else {
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t * decoded = decode_png_data(png_data, png_data_size);

    if(dsc->src_type == LV_IMAGE_SRC_FILE) lv_free((void *)png_data);

    if(!decoded) {
        LV_LOG_WARN("Error decoding PNG");
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_INVALID;
    }

    lv_draw_buf_t * adjusted = lv_image_decoder_post_process(dsc, decoded);
    if(adjusted == NULL) {
        lv_draw_buf_destroy(decoded);
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_INVALID;
    }

    /*The adjusted draw buffer is newly allocated.*/
    if(adjusted != decoded) {
        lv_draw_buf_destroy(decoded);
        decoded = adjusted;
    }

    dsc->decoded = decoded;

    if(dsc->args.no_cache) {
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_OK;
    }

    /*If the image cache is disabled, just return the decoded image*/
    if(!lv_image_cache_is_enabled()) {
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_OK;
    }

    /*Add the decoded image to the cache*/
    lv_image_cache_data_t search_key;
    search_key.src_type = dsc->src_type;
    search_key.src = dsc->src;
    search_key.slot.size = decoded->data_size;

    lv_cache_entry_t * entry = lv_image_decoder_add_to_cache(decoder, &search_key, decoded, NULL);

    if(entry == NULL) {
        LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
        return LV_RESULT_INVALID;
    }
    dsc->cache_entry = entry;

    LV_PROFILER_DECODER_END_TAG("lv_lodepng_decoder_open");
    return LV_RESULT_OK;    /*If not returned earlier then it failed*/
}

/**
 * Close PNG image and free data
 * @param decoder   pointer to the decoder where this function belongs
 * @param dsc       decoded image descriptor
 * @return          LV_RESULT_OK: no error; LV_RESULT_INVALID: can't open the image
 */
static void decoder_close(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);

    if(dsc->args.no_cache ||
       !lv_image_cache_is_enabled()) lv_draw_buf_destroy((lv_draw_buf_t *)dsc->decoded);
}

static lv_draw_buf_t * decode_png_data(const void * png_data, size_t png_data_size)
{
    unsigned png_width;             /*Not used, just required by the decoder*/
    unsigned png_height;            /*Not used, just required by the decoder*/
    lv_draw_buf_t * decoded = NULL;

    uint32_t totalStart = lv_tick_get();
    uint32_t decodeStart = lv_tick_get();

    /*Decode the image in ARGB8888 */
    unsigned error = lodepng_decode32((unsigned char **)&decoded, &png_width, &png_height, png_data, png_data_size);

    uint32_t decodeMs = lv_tick_elaps(decodeStart);

    if(error) {
        if(decoded != NULL)  lv_draw_buf_destroy(decoded);
        return NULL;
    }

    /*Measure the original ARGB8888 R/B channel swap separately.*/
    uint32_t convertStart = lv_tick_get();

    /*Convert the image to the system's color depth*/
    convert_color_depth(decoded->data,  png_width * png_height);

    uint32_t convertMs = lv_tick_elaps(convertStart);
    uint32_t totalMs = lv_tick_elaps(totalStart);

    bk_printf(TAG "[PNG_PERF] mode=ARGB8888 size=%ux%u decode32=%lu ms rb_swap=%lu ms total=%lu ms\\n",
        png_width,
        png_height,
        (unsigned long)decodeMs,
        (unsigned long)convertMs,
        (unsigned long)totalMs);

    return decoded;
}

/**
 * If the display is not in 32 bit format (ARGB888) then convert the image to the current color depth
 * @param img the ARGB888 image
 * @param px_cnt number of pixels in `img`
 */
static void convert_color_depth(uint8_t * img_p, uint32_t px_cnt)
{
    lv_color32_t * img_argb = (lv_color32_t *)img_p;
    uint32_t i;
    for(i = 0; i < px_cnt; i++) {
        uint8_t blue = img_argb[i].blue;
        img_argb[i].blue = img_argb[i].red;
        img_argb[i].red = blue;
    }
}
#endif /*UI_PRENDERING_ENABLE*/
#endif /*LV_USE_LODEPNG*/
