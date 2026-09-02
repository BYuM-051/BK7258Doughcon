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

/*
 * Custom native-output entry point implemented in lodepng.c.
 *
 * It returns LodePNG's decompressed/post-processed native PNG pixel buffer
 * without allocating an intermediate ARGB8888/RGBA8888 lv_draw_buf_t.
 *
 * The buffer format is described by colorType/bitDepth. Palette entries are
 * copied into this structure so they remain valid after LodePNGState cleanup.
 */
typedef struct
{
    unsigned char * data;

    unsigned width;
    unsigned height;

    LodePNGColorType colorType;
    unsigned bitDepth;
    unsigned interlaceMethod;

    unsigned keyDefined;
    unsigned keyR;
    unsigned keyG;
    unsigned keyB;

    size_t paletteSize;
    unsigned char palette[256 * 4];
} lodepng_native_image_t;

unsigned lodepng_decode_native(lodepng_native_image_t * out,
                               const unsigned char * in,
                               size_t insize);

void lodepng_native_image_cleanup(lodepng_native_image_t * image);
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
 * Decode PNG from LodePNG's native post-processed pixel buffer directly to RGB565A8.
 *
 * No intermediate ARGB8888/RGBA8888 lv_draw_buf_t is allocated.
 *
 * Supported native PNG color formats:
 *
 *     LCT_PALETTE    1/2/4/8 bit
 *     LCT_GREY       1/2/4/8/16 bit
 *     LCT_RGB        8/16 bit
 *     LCT_GREY_ALPHA 8/16 bit
 *     LCT_RGBA       8/16 bit
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
    lodepng_native_image_t nativeImage;
    lv_memzero(&nativeImage, sizeof(nativeImage));

    uint32_t totalStart = lv_tick_get();
    uint32_t decodeStart = lv_tick_get();

    unsigned error = lodepng_decode_native(
        &nativeImage,
        png_data,
        png_data_size
    );

    uint32_t decodeMs = lv_tick_elaps(decodeStart);

    if(error)
    {
        lodepng_native_image_cleanup(&nativeImage);

        LV_LOG_WARN(
            "lodepng_decode_native failed: error=%u (%s)",
            error,
            lodepng_error_text(error)
        );

        return NULL;
    }

    const unsigned png_width = nativeImage.width;
    const unsigned png_height = nativeImage.height;

    if(nativeImage.data == NULL ||
       png_width == 0 ||
       png_height == 0)
    {
        lodepng_native_image_cleanup(&nativeImage);
        return NULL;
    }

    uint32_t stage565Start = lv_tick_get();

    /*
     * Allocate only the final image-cache buffer as RGB565A8.
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
        lodepng_native_image_cleanup(&nativeImage);
        return NULL;
    }

    /*
     * Clear padding bytes as stride can be larger than width * 2
     * because of LV_DRAW_BUF_STRIDE_ALIGN.
     */
    lv_memzero(decoded->data, decoded->data_size);

    const uint32_t dstStride = decoded->header.stride;

    uint8_t * colorData = decoded->data;

    /*
     * RGB565A8's alpha plane starts after the complete RGB565 plane.
     */
    uint8_t * alphaData =
        decoded->data +
        (dstStride * png_height);

    const uint32_t alphaStride = dstStride / 2;

    uint32_t convertStart = lv_tick_get();

    if(nativeImage.colorType == LCT_PALETTE)
    {
        if(nativeImage.bitDepth != 1 &&
           nativeImage.bitDepth != 2 &&
           nativeImage.bitDepth != 4 &&
           nativeImage.bitDepth != 8)
        {
            lv_draw_buf_destroy(decoded);
            lodepng_native_image_cleanup(&nativeImage);
            return NULL;
        }

        const uint8_t * srcData = nativeImage.data;
        const unsigned bitDepth = nativeImage.bitDepth;
        const unsigned mask = (1u << bitDepth) - 1u;

        for(uint32_t y = 0; y < png_height; y++)
        {
            uint16_t * colorRow =
                (uint16_t *)(colorData +
                             (y * dstStride));

            uint8_t * alphaRow =
                alphaData +
                (y * alphaStride);

            for(uint32_t x = 0; x < png_width; x++)
            {
                const size_t pixelIndex =
                    ((size_t)y * png_width) + x;

                unsigned paletteIndex;

                if(bitDepth == 8)
                {
                    paletteIndex = srcData[pixelIndex];
                }
                else
                {
                    const size_t bitIndex =
                        pixelIndex * bitDepth;

                    const size_t byteIndex =
                        bitIndex >> 3u;

                    const unsigned bitOffset =
                        (unsigned)(bitIndex & 7u);

                    const unsigned shift =
                        8u - bitDepth - bitOffset;

                    paletteIndex =
                        (srcData[byteIndex] >> shift) & mask;
                }

                if(paletteIndex >= nativeImage.paletteSize)
                {
                    lv_draw_buf_destroy(decoded);
                    lodepng_native_image_cleanup(&nativeImage);
                    return NULL;
                }

                const uint8_t * palette =
                    &nativeImage.palette[paletteIndex * 4u];

                const uint8_t r = palette[0];
                const uint8_t g = palette[1];
                const uint8_t b = palette[2];
                const uint8_t a = palette[3];

                colorRow[x] =
                    ((uint16_t)(r & 0xF8) << 8) |
                    ((uint16_t)(g & 0xFC) << 3) |
                    ((uint16_t)(b & 0xF8) >> 3);

                alphaRow[x] = a;
            }
        }
    }
    else if(nativeImage.colorType == LCT_RGB)
    {
        if(nativeImage.bitDepth != 8 &&
           nativeImage.bitDepth != 16)
        {
            lv_draw_buf_destroy(decoded);
            lodepng_native_image_cleanup(&nativeImage);
            return NULL;
        }

        const uint8_t * srcData = nativeImage.data;
        const uint32_t srcBytesPerPixel =
            nativeImage.bitDepth == 8 ? 3u : 6u;

        for(uint32_t y = 0; y < png_height; y++)
        {
            const uint8_t * srcRow =
                srcData +
                ((size_t)y * png_width * srcBytesPerPixel);

            uint16_t * colorRow =
                (uint16_t *)(colorData +
                             (y * dstStride));

            uint8_t * alphaRow =
                alphaData +
                (y * alphaStride);

            for(uint32_t x = 0; x < png_width; x++)
            {
                const uint8_t * srcPixel =
                    srcRow +
                    ((size_t)x * srcBytesPerPixel);

                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t a = 255;

                if(nativeImage.bitDepth == 8)
                {
                    r = srcPixel[0];
                    g = srcPixel[1];
                    b = srcPixel[2];

                    if(nativeImage.keyDefined &&
                       r == nativeImage.keyR &&
                       g == nativeImage.keyG &&
                       b == nativeImage.keyB)
                    {
                        a = 0;
                    }
                }
                else
                {
                    const unsigned r16 =
                        ((unsigned)srcPixel[0] << 8u) |
                        srcPixel[1];

                    const unsigned g16 =
                        ((unsigned)srcPixel[2] << 8u) |
                        srcPixel[3];

                    const unsigned b16 =
                        ((unsigned)srcPixel[4] << 8u) |
                        srcPixel[5];

                    r = srcPixel[0];
                    g = srcPixel[2];
                    b = srcPixel[4];

                    if(nativeImage.keyDefined &&
                       r16 == nativeImage.keyR &&
                       g16 == nativeImage.keyG &&
                       b16 == nativeImage.keyB)
                    {
                        a = 0;
                    }
                }

                colorRow[x] =
                    ((uint16_t)(r & 0xF8) << 8) |
                    ((uint16_t)(g & 0xFC) << 3) |
                    ((uint16_t)(b & 0xF8) >> 3);

                alphaRow[x] = a;
            }
        }
    }
    else if(nativeImage.colorType == LCT_RGBA)
    {
        if(nativeImage.bitDepth != 8 &&
           nativeImage.bitDepth != 16)
        {
            lv_draw_buf_destroy(decoded);
            lodepng_native_image_cleanup(&nativeImage);
            return NULL;
        }

        const uint8_t * srcData = nativeImage.data;
        const uint32_t srcBytesPerPixel =
            nativeImage.bitDepth == 8 ? 4u : 8u;

        for(uint32_t y = 0; y < png_height; y++)
        {
            const uint8_t * srcRow =
                srcData +
                ((size_t)y * png_width * srcBytesPerPixel);

            uint16_t * colorRow =
                (uint16_t *)(colorData +
                             (y * dstStride));

            uint8_t * alphaRow =
                alphaData +
                (y * alphaStride);

            for(uint32_t x = 0; x < png_width; x++)
            {
                const uint8_t * srcPixel =
                    srcRow +
                    ((size_t)x * srcBytesPerPixel);

                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t a;

                if(nativeImage.bitDepth == 8)
                {
                    r = srcPixel[0];
                    g = srcPixel[1];
                    b = srcPixel[2];
                    a = srcPixel[3];
                }
                else
                {
                    r = srcPixel[0];
                    g = srcPixel[2];
                    b = srcPixel[4];
                    a = srcPixel[6];
                }

                colorRow[x] =
                    ((uint16_t)(r & 0xF8) << 8) |
                    ((uint16_t)(g & 0xFC) << 3) |
                    ((uint16_t)(b & 0xF8) >> 3);

                alphaRow[x] = a;
            }
        }
    }
    else if(nativeImage.colorType == LCT_GREY)
    {
        if(nativeImage.bitDepth != 1 &&
           nativeImage.bitDepth != 2 &&
           nativeImage.bitDepth != 4 &&
           nativeImage.bitDepth != 8 &&
           nativeImage.bitDepth != 16)
        {
            lv_draw_buf_destroy(decoded);
            lodepng_native_image_cleanup(&nativeImage);
            return NULL;
        }

        const uint8_t * srcData = nativeImage.data;
        const unsigned bitDepth = nativeImage.bitDepth;
        const unsigned highest =
            bitDepth < 16 ? ((1u << bitDepth) - 1u) : 65535u;

        for(uint32_t y = 0; y < png_height; y++)
        {
            uint16_t * colorRow =
                (uint16_t *)(colorData +
                             (y * dstStride));

            uint8_t * alphaRow =
                alphaData +
                (y * alphaStride);

            for(uint32_t x = 0; x < png_width; x++)
            {
                const size_t pixelIndex =
                    ((size_t)y * png_width) + x;

                unsigned greyValue;

                if(bitDepth == 8)
                {
                    greyValue = srcData[pixelIndex];
                }
                else if(bitDepth == 16)
                {
                    const size_t byteIndex =
                        pixelIndex * 2u;

                    greyValue =
                        ((unsigned)srcData[byteIndex] << 8u) |
                        srcData[byteIndex + 1u];
                }
                else
                {
                    const size_t bitIndex =
                        pixelIndex * bitDepth;

                    const size_t byteIndex =
                        bitIndex >> 3u;

                    const unsigned bitOffset =
                        (unsigned)(bitIndex & 7u);

                    const unsigned shift =
                        8u - bitDepth - bitOffset;

                    greyValue =
                        (srcData[byteIndex] >> shift) &
                        ((1u << bitDepth) - 1u);
                }

                const uint8_t grey =
                    (uint8_t)((greyValue * 255u) / highest);

                uint8_t a = 255;

                if(nativeImage.keyDefined &&
                   greyValue == nativeImage.keyR)
                {
                    a = 0;
                }

                colorRow[x] =
                    ((uint16_t)(grey & 0xF8) << 8) |
                    ((uint16_t)(grey & 0xFC) << 3) |
                    ((uint16_t)(grey & 0xF8) >> 3);

                alphaRow[x] = a;
            }
        }
    }
    else if(nativeImage.colorType == LCT_GREY_ALPHA)
    {
        if(nativeImage.bitDepth != 8 &&
           nativeImage.bitDepth != 16)
        {
            lv_draw_buf_destroy(decoded);
            lodepng_native_image_cleanup(&nativeImage);
            return NULL;
        }

        const uint8_t * srcData = nativeImage.data;
        const uint32_t srcBytesPerPixel =
            nativeImage.bitDepth == 8 ? 2u : 4u;

        for(uint32_t y = 0; y < png_height; y++)
        {
            const uint8_t * srcRow =
                srcData +
                ((size_t)y * png_width * srcBytesPerPixel);

            uint16_t * colorRow =
                (uint16_t *)(colorData +
                             (y * dstStride));

            uint8_t * alphaRow =
                alphaData +
                (y * alphaStride);

            for(uint32_t x = 0; x < png_width; x++)
            {
                const uint8_t * srcPixel =
                    srcRow +
                    ((size_t)x * srcBytesPerPixel);

                uint8_t grey;
                uint8_t a;

                if(nativeImage.bitDepth == 8)
                {
                    grey = srcPixel[0];
                    a = srcPixel[1];
                }
                else
                {
                    grey = srcPixel[0];
                    a = srcPixel[2];
                }

                colorRow[x] =
                    ((uint16_t)(grey & 0xF8) << 8) |
                    ((uint16_t)(grey & 0xFC) << 3) |
                    ((uint16_t)(grey & 0xF8) >> 3);

                alphaRow[x] = a;
            }
        }
    }
    else
    {
        lv_draw_buf_destroy(decoded);
        lodepng_native_image_cleanup(&nativeImage);
        return NULL;
    }

    uint32_t convertMs = lv_tick_elaps(convertStart);

    const unsigned decodedColorType = (unsigned)nativeImage.colorType;
    const unsigned decodedBitDepth = nativeImage.bitDepth;

    /*
     * The native PNG pixel allocation is no longer needed.
     * Only RGB565A8 remains resident and can enter the LVGL image cache.
     */
    lodepng_native_image_cleanup(&nativeImage);

    uint32_t stage565Ms = lv_tick_elaps(stage565Start);
    uint32_t totalMs = lv_tick_elaps(totalStart);

    bk_printf(TAG "[PNG_PERF] mode=RGB565A8_NATIVE size=%ux%u type=%u depth=%u decode_native=%lu ms convert_loop=%lu ms 565A8_stage=%lu ms total=%lu ms\\n",
        png_width,
        png_height,
        decodedColorType,
        decodedBitDepth,
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
