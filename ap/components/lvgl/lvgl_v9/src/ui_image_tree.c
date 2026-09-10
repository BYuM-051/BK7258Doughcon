/**
 * @file ui_image_tree.c
 *
 * flat "/images/foo.png" -> tree "/images/dNNN/foo.png" 변환.
 * uiImageTreeMap[] 테이블이 링크되는 유일한 TU 이다.
 */

/*********************
 *      INCLUDES
 *********************/
#include <string.h>
#include <stdio.h>

#include "../lvgl.h"
#include "draw/lv_draw_image.h"   /* lv_image_src_get_type() */

#include "ui_image_tree.h"
#include "ui_image_tree_map.h"

/*********************
 *      DEFINES
 *********************/
#define TAG "[ui_image_tree.c] "

#define UI_IMAGE_PATH_PREFIX      "/images/"
#define UI_IMAGE_PATH_PREFIX_LEN  8

/* bk_printf 원형은 ap/include/components/system.h 에 있으나 lvgl 쪽에서
 * 그 헤더를 직접 포함하지 않으므로(다른 lvgl 파일들과 동일) 여기서 선언만 한다. */
extern void bk_printf(const char *fmt, ...);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

const char *ui_image_tree_resolve_path_ex(const void *src, char *pathBuffer, size_t pathBufferSize, bool quiet)
{
    if(src == NULL) {return NULL;}

    if(pathBuffer == NULL || pathBufferSize == 0) { return (const char *)src; }

    if(lv_image_src_get_type(src) != LV_IMAGE_SRC_FILE)
    {
        return (const char *)src;
    }

    const char *srcPath = (const char *)src;

    if(strncmp(srcPath, UI_IMAGE_PATH_PREFIX, UI_IMAGE_PATH_PREFIX_LEN) != 0) { return srcPath; }

    const char *fileName = srcPath + UI_IMAGE_PATH_PREFIX_LEN;

    if(strchr(fileName, '/') != NULL)
    {
        return srcPath;
    }


    for(size_t i = 0; i < UI_IMAGE_TREE_MAP_COUNT; i++)
    {
        if(strcmp(fileName, uiImageTreeMap[i].fileName) == 0)
        {
            int result = snprintf(pathBuffer, pathBufferSize, "/images/d%03u/%s", (unsigned int)uiImageTreeMap[i].dirIndex, fileName);

            if(result < 0 || (size_t)result >= pathBufferSize)
            {
                bk_printf(TAG "[TREE] path buffer too small src=[%s]\n", srcPath);
                return srcPath;
            }

            return pathBuffer;
        }
    }

    if(!quiet) { bk_printf(TAG "[TREE] image map not found src=[%s]\n", srcPath); }

    return srcPath;
}

const char *ui_image_tree_resolve_path(const void *src, char *pathBuffer, size_t pathBufferSize)
{
    return ui_image_tree_resolve_path_ex(src, pathBuffer, pathBufferSize, false);
}
