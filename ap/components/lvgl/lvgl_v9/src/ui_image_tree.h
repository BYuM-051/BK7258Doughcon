/**
 * @file ui_image_tree.h
 *
 * Flat "/images/foo.png" 경로를 tree 디렉터리 경로 "/images/dNNN/foo.png" 로
 * 변환하는 공용 resolver.
 *
 * 이미지 파일을 한 디렉터리에 660개 몰아넣지 않고 dNNN 하위로 쪼개면서
 * decoder / widget 쪽에 각각 static 사본이 생겼는데, image cache 의 key 는
 * "decoder 에 넘어간 경로" 이므로 app 레이어가 flat 경로로 lv_image_cache_drop()
 * 을 호출하면 아무것도 지워지지 않는다. 변환 규칙을 한 곳으로 모아 map 테이블도
 * 한 번만 링크되게 한다.
 */
#ifndef UI_IMAGE_TREE_H
#define UI_IMAGE_TREE_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_IMAGE_TREE_PATH_MAX    256

/**
 * src 를 tree 경로로 변환한다.
 *
 * 아래 경우에는 변환 없이 src 를 그대로 돌려준다.
 *   - src == NULL
 *   - FILE 소스가 아님 (VARIABLE / SYMBOL)
 *   - "/images/" 로 시작하지 않음
 *   - 이미 "/images/d003/foo.png" 처럼 tree 경로임
 *   - map 에 없는 파일명 (quiet == false 이면 로그 1줄)
 *
 * @param src            원본 소스 (보통 const char * 파일 경로)
 * @param pathBuffer     변환 결과를 담을 버퍼. 최소 UI_IMAGE_TREE_PATH_MAX 권장
 * @param pathBufferSize pathBuffer 크기
 * @param quiet          true 면 map miss 로그를 남기지 않는다
 * @return               변환된 경로 또는 원본 src
 */
const char *ui_image_tree_resolve_path_ex(const void *src, char *pathBuffer, size_t pathBufferSize, bool quiet);

/** ui_image_tree_resolve_path_ex(src, buf, size, false) */
const char *ui_image_tree_resolve_path(const void *src, char *pathBuffer, size_t pathBufferSize);

#ifdef __cplusplus
}
#endif

#endif /* UI_IMAGE_TREE_H */
