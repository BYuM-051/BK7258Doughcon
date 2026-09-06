#!/usr/bin/env python3

from pathlib import Path
import math
import shutil


# ============================================================
# Path configuration
# ============================================================

REPO_ROOT = Path(__file__).resolve().parent

BASE_DIR = (
    REPO_ROOT /
    "projects/lvgl/korea_test"
)

SOURCE_IMAGE_ROOT = (
    BASE_DIR /
    "vfs_file/images"
)

TREE_IMAGE_ROOT = (
    BASE_DIR /
    "tree_file/images"
)

OUTPUT_HEADER = (
    REPO_ROOT /
    "ap/components/lvgl/lvgl_v9/src/ui_image_tree_map.h"
)


# ============================================================
# Directory configuration
# ============================================================

DIR_PREFIX = "d"
DIR_DIGITS = 3


# ============================================================
# Clean generated outputs
# ============================================================

def clean_generated_outputs():
    if TREE_IMAGE_ROOT.exists():
        print(
            f"[CLEAN] remove tree root: "
            f"{TREE_IMAGE_ROOT}"
        )

        shutil.rmtree(
            TREE_IMAGE_ROOT
        )

    if OUTPUT_HEADER.exists():
        print(
            f"[CLEAN] remove header: "
            f"{OUTPUT_HEADER}"
        )

        OUTPUT_HEADER.unlink()


# ============================================================
# Collect source images
# ============================================================

def collect_source_files():
    if not SOURCE_IMAGE_ROOT.exists():
        raise RuntimeError(
            f"source image directory not found:\n"
            f"{SOURCE_IMAGE_ROOT}"
        )

    #
    # vfs_file/images 바로 아래의 파일만 사용한다.
    #
    # 하위 디렉터리는 무시.
    #
    files = [
        path
        for path in SOURCE_IMAGE_ROOT.iterdir()
        if path.is_file()
    ]

    #
    # 실행할 때마다 동일한 파일이 동일한 bucket으로
    # 배치되도록 정렬.
    #
    files.sort(
        key=lambda path: path.name.lower()
    )

    return files


# ============================================================
# Generate C header
# ============================================================

def generate_header(
    entries,
    file_count,
    dir_count,
    files_per_dir
):
    OUTPUT_HEADER.parent.mkdir(
        parents=True,
        exist_ok=True
    )

    with OUTPUT_HEADER.open(
        "w",
        encoding="utf-8",
        newline="\n"
    ) as f:
        f.write(
            "#ifndef UI_IMAGE_TREE_MAP_H\n"
        )

        f.write(
            "#define UI_IMAGE_TREE_MAP_H\n\n"
        )

        f.write(
            "#include <stdint.h>\n"
        )

        f.write(
            "#include <stddef.h>\n\n"
        )

        f.write(
            f"#define UI_IMAGE_TREE_FILE_COUNT "
            f"{file_count}U\n"
        )

        f.write(
            f"#define UI_IMAGE_TREE_DIR_COUNT "
            f"{dir_count}U\n"
        )

        f.write(
            f"#define UI_IMAGE_TREE_FILES_PER_DIR "
            f"{files_per_dir}U\n\n"
        )

        f.write(
            "typedef struct\n"
            "{\n"
            "    const char *fileName;\n"
            "    uint16_t dirIndex;\n"
            "} ui_image_tree_map_t;\n\n"
        )

        f.write(
            "static const ui_image_tree_map_t "
            "uiImageTreeMap[] =\n"
            "{\n"
        )

        for file_name, dir_index in entries:
            f.write(
                f'    {{ "{file_name}", '
                f'{dir_index}U }},\n'
            )

        f.write(
            "};\n\n"
        )

        f.write(
            "#define UI_IMAGE_TREE_MAP_COUNT "
            "(sizeof(uiImageTreeMap) / "
            "sizeof(uiImageTreeMap[0]))\n\n"
        )

        f.write(
            "#endif /* UI_IMAGE_TREE_MAP_H */\n"
        )


# ============================================================
# Build tree directory
# ============================================================

def create_tree(files):
    file_count = len(files)

    if file_count == 0:
        raise RuntimeError(
            f"no image files found:\n"
            f"{SOURCE_IMAGE_ROOT}"
        )

    #
    # N개의 파일에 대해:
    #
    # directory count ~= sqrt(N)
    # files per directory ~= sqrt(N)
    #
    dir_count = math.ceil(
        math.sqrt(file_count)
    )

    files_per_dir = math.ceil(
        file_count / dir_count
    )

    print()
    print(
        f"[INFO] source      : "
        f"{SOURCE_IMAGE_ROOT}"
    )

    print(
        f"[INFO] destination : "
        f"{TREE_IMAGE_ROOT}"
    )

    print(
        f"[INFO] header      : "
        f"{OUTPUT_HEADER}"
    )

    print(
        f"[INFO] file count  : "
        f"{file_count}"
    )

    print(
        f"[INFO] dir count   : "
        f"{dir_count}"
    )

    print(
        f"[INFO] files/dir   : "
        f"<= {files_per_dir}"
    )

    print()

    #
    # tree_file/images 재생성
    #
    TREE_IMAGE_ROOT.mkdir(
        parents=True,
        exist_ok=True
    )

    #
    # d000, d001, ...
    #
    for dir_index in range(dir_count):
        directory = (
            TREE_IMAGE_ROOT /
            f"{DIR_PREFIX}"
            f"{dir_index:0{DIR_DIGITS}d}"
        )

        directory.mkdir(
            parents=True,
            exist_ok=True
        )

    entries = []

    #
    # 원본 vfs_file/images 파일들은 건드리지 않고
    # tree_file/images 아래로 복사한다.
    #
    for index, source_path in enumerate(files):
        dir_index = (
            index // files_per_dir
        )

        #
        # rounding 방어
        #
        if dir_index >= dir_count:
            dir_index = dir_count - 1

        directory_name = (
            f"{DIR_PREFIX}"
            f"{dir_index:0{DIR_DIGITS}d}"
        )

        destination_path = (
            TREE_IMAGE_ROOT /
            directory_name /
            source_path.name
        )

        shutil.copy2(
            source_path,
            destination_path
        )

        entries.append(
            (
                source_path.name,
                dir_index
            )
        )

        print(
            f"[COPY] "
            f"{source_path.name} "
            f"-> {directory_name}/"
        )

    #
    # 헤더의 strcmp 검색 결과 역시
    # 항상 deterministic하도록 정렬
    #
    entries.sort(
        key=lambda item: item[0].lower()
    )

    generate_header(
        entries,
        file_count,
        dir_count,
        files_per_dir
    )

    return (
        file_count,
        dir_count,
        files_per_dir
    )


# ============================================================
# Main
# ============================================================

def main():
    print(
        "========================================"
    )

    print(
        " UI Image Tree Directory Generator"
    )

    print(
        "========================================"
    )

    #
    # 실행할 때마다 기존 generated 결과는 전부 삭제
    #
    clean_generated_outputs()

    #
    # 원본 flat image 목록 수집
    #
    files = collect_source_files()

    #
    # tree directory + header 생성
    #
    (
        file_count,
        dir_count,
        files_per_dir
    ) = create_tree(files)

    print()
    print(
        "========================================"
    )

    print(
        "[DONE]"
    )

    print(
        f"files       : {file_count}"
    )

    print(
        f"directories : {dir_count}"
    )

    print(
        f"files/dir   : <= {files_per_dir}"
    )

    print(
        f"tree root   : {TREE_IMAGE_ROOT}"
    )

    print(
        f"header      : {OUTPUT_HEADER}"
    )

    print(
        "========================================"
    )


if __name__ == "__main__":
    main()