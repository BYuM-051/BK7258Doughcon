#!/bin/bash

# bk7258 project build script for wsl environment
# project will be built in the wsl environment and output will be copied to the windows host to burn
# 2026 08 20 BYuM @ 연구개발부
# tgkim@neurosys.co.kr
# 

set -e

FULL_PROJECT="lvgl/korea_test"
NOCLEAN=false

while [ $# -gt 0 ]; do
    case "$1" in
        --help)
            echo "Usage: $0 [--noclean] [--project <project_name>]"
            echo "  --noclean : Do not clean build artifacts after build"
            echo "  --project <project_name> : Specify the project name to build"
            echo "  --lfsSize <size> : Specify the size of the littleFS partition"
            exit 0
            ;;

        --project)
            if [ -z "$2" ]; then
                echo "Error: --project option requires a project name"
                exit 1
            fi

            FULL_PROJECT="$2"
            shift 2
            ;;

        --noclean)
            NOCLEAN=true
            shift
            ;;

        --lfsSize)
            if [ -z "$2" ]; then
                echo "Error: --lfsSize option requires a size value"
                exit 1
            fi

            SIZE="$2"
            shift 2
            ;;

        *)
            echo "Error: Unknown option: $1"
            exit 1
            ;;
    esac
done

OUTPUT_DIR="/mnt/c/workspace/2026/beken/output"
PROJECT="${FULL_PROJECT##*/}"
PARTITION_TABLE="build/bk7258/${PROJECT}/partitions/partitions.csv"
BUILD_OUTPUT="build/bk7258/${PROJECT}/package/all-app.bin"
LITTLEFS_SOURCE="projects/${FULL_PROJECT}/vfs_file/"
LITTLEFS_OUTPUT="littlefs.bin"
NOTICED=false

echo "[SHELL]===========================[START] PROJECT=${FULL_PROJECT}"

echo "[SHELL]===========================[BUILD] PROJECT=${PROJECT}"
make bk7258 PROJECT="${FULL_PROJECT}" > /dev/null 2>&1
echo "[SHELL]===========================[BUILD_DONE]"

echo "[SHELL]===========================[Make littleFS]"
#validate littleFS size
#check the file source directory and match -s option in mklittlefs command
if [ -z "$SIZE" ]; then
    SIZE=$(find "${LITTLEFS_SOURCE}" -type f -printf '%s\n' | awk '{sum += $1} END {print sum+4096}')
fi
while true; do
    rm -f littlefs.bin

    LITTLEFS_TRY=$(
        ap/components/littlefs/mkimg/mklittlefs \
        -c "${LITTLEFS_SOURCE}" \
        -b 4096 \
        -p 256 \
        -s "${SIZE}" \
        "${LITTLEFS_OUTPUT}" 2>&1
    )

    if echo "$LITTLEFS_TRY" | grep -q "error adding file!" || echo "$LITTLEFS_TRY" | grep -q "No more free space"; then # 나도 이러고싶진 않았어. done message가 없는데 어캄.
        SIZE=$((SIZE + 4096))
        if [ "$NOTICED" = false ]; then
        NOTICED=true
        echo "[SHELL]===========================[Make littleFS]"
        echo "[SHELL]Size too small, increasing to ${SIZE} and retrying..."
        echo "[SHElL]--TIPS-- one . per 4KB increase"
        fi
        echo -n "."
    else
        if [ "$NOTICED" = true ]; then
            echo ""
        fi
        break
    fi
done
echo "[SHELL]===========================[Make littleFS Done]"

echo "[SHELL]===========================[COPY OUTPUT] ${OUTPUT_DIR}/"
mkdir -p "${OUTPUT_DIR}"
cp "${BUILD_OUTPUT}" "${OUTPUT_DIR}/"
cp "${PARTITION_TABLE}" "${OUTPUT_DIR}/"
cp "${LITTLEFS_OUTPUT}" "${OUTPUT_DIR}/"
echo "[SHELL]===========================[COPY_OUTPUT_DONE]"

if [ "$NOCLEAN" = false ]; then
    make clean > /dev/null 2>&1
    rm -rf ./build
    rm -f ./littlefs.bin

    echo "[SHELL]===========================[CLEAN_TEMP_DONE]"
else
    echo "[SHELL]===========================[SKIP_CLEAN_TEMP]"
fi

echo "[SHELL]===========================[BUILD_COMPLETE]"
echo "[SHELL]===========================[ALL_APP] ${OUTPUT_DIR}/all-app.bin"
echo "[SHELL]===========================[PARTITIONS] ${OUTPUT_DIR}/partitions.csv"
echo "[SHELL]===========================[LITTLEFS] ${OUTPUT_DIR}/littlefs.bin"
echo "[SHELL]===========================[LITTLEFS_SIZE] ${SIZE}"