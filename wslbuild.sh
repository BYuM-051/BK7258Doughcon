#!/bin/bash

# bk7258 project build script for wsl environment
# project will be built in the wsl environment and output will be copied to the windows host to burn
# 2026 08 19 BYuM @ 연구개발부
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
LITTLEFS_OUTPUT="littlefs.bin"

echo "[SHELL]===========================[START] PROJECT=${FULL_PROJECT}"


echo "[SHELL]===========================[BUILD] PROJECT=${PROJECT}"

make bk7258 PROJECT="${FULL_PROJECT}"

echo "[SHELL]===========================[BUILD_DONE]"

echo "[SHELL]===========================[Make littleFS]"

#TODO : validate littleFS size
#check the file source directory and match -s option in mklittlefs command
ap/components/littlefs/mkimg/mklittlefs \
-c projects/${FULL_PROJECT}/vfs_file/ \
-b 4096 \
-p 256 \
-s 4096 \
"${LITTLEFS_OUTPUT}"

echo "[SHELL]===========================[Make littleFS Done]"

echo "[SHELL]===========================[COPY OUTPUT] ${OUTPUT_DIR}/"

mkdir -p "${OUTPUT_DIR}"
cp "${BUILD_OUTPUT}" "${OUTPUT_DIR}/"
cp "${PARTITION_TABLE}" "${OUTPUT_DIR}/"
cp "${LITTLEFS_OUTPUT}" "${OUTPUT_DIR}/"

if [ "$NOCLEAN" = false ]; then
    echo "[SHELL]===========================[CLEAN]"

    make clean
    rm -rf ./build
    rm -f ./littlefs.bin

    echo "[SHELL]===========================[CLEAN_DONE]"
fi

echo "[SHELL]===========================[DONE] ${OUTPUT_DIR}/all-app.bin"