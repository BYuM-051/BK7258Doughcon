#!/bin/bash

# bk7258 project build script for wsl environment
# project will be built in the wsl environment and output will be copied to the windows host to burn
# 2026 08 20 BYuM @ 연구개발부
# tgkim@neurosys.co.kr
# version 26.08.20.04

set -e

FULL_PROJECT=""
NOCLEAN=false
LFSBUILD=true

while [ $# -gt 0 ]; do
    case "$1" in
        --help)
            echo "Usage: $0 --project <project_path> [--noclean] [--lfsSize <size>] [--output <directory>]"
            echo "  --noclean : Do not clean build artifacts after build"
            echo "  --project <project_path> : Specify the project path to build"
            echo "  --lfsSize <size> : Specify the size of the littleFS partition in bytes [0 to skip littleFS build / default : autocalc]"
            echo "  --output <directory> : Specify the output directory for build artifacts [default: /mnt/c/build_output/<project_name>]"
            exit 0
            ;;

        --project)
            if [ -z "$2" ]; then
                echo "Error: --project option requires a project path"
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

            if ! [[ "$SIZE" =~ ^[0-9]+$ ]]; then
                echo "Error: --lfsSize value must be a positive integer"
                exit 1
                elif [ "$SIZE" == "0" ]; then
                LFSBUILD=false
            fi
            shift 2
            ;;

        --output)
            if [ -z "$2" ]; then
                echo "Error: --output option requires a directory path"
                exit 1
            fi

            OUTPUT_DIR="/mnt/c/${2}/"
            shift 2
            ;;

        *)
            echo "Error: Unknown option: $1"
            exit 1
            ;;
    esac
done

if [ -z "$FULL_PROJECT" ]; then
    echo "Error: --project option is required"
    exit 1
fi

if [ -z "$OUTPUT_DIR" ]; then
    OUTPUT_DIR="/mnt/c/build_output/${FULL_PROJECT}"
fi
PROJECT="${FULL_PROJECT##*/}"
PARTITION_TABLE="build/bk7258/${PROJECT}/partitions/partitions.csv"
BUILD_OUTPUT="build/bk7258/${PROJECT}/package/all-app.bin"
LITTLEFS_SOURCE="projects/${FULL_PROJECT}/vfs_file/"
LITTLEFS_OUTPUT="littlefs.bin"
NOTICED=false

echo "[BUILDER]===========================[START]==========================="
echo "[BUILDER] PROJECT_PATH=${FULL_PROJECT}"

echo "[BUILDER]=[BUILD]==========================="
echo "[BUILDER] [BUILD] PROJECT=${PROJECT}"
make bk7258 PROJECT="${FULL_PROJECT}" > /dev/null 2>&1
echo "[BUILDER]=[BUILD_DONE]==========================="

if [ "$LFSBUILD" = true ]; then
echo "[BUILDER]=[Make littleFS]==========================="
#validate littleFS size
#check the file source directory and match -s option in mklittlefs command
if [ -z "$SIZE" ]; then
    SIZE=$(find "${LITTLEFS_SOURCE}" -type f -printf '%s\n' | awk '{sum += $1} END {print sum+4096}') 
fi
SIZE=$(( (SIZE + 4095) / 4096 * 4096 )) #normalization with 4KB unit

echo "[BUILDER] [Make littleFS] INPUT_SIZE=${SIZE}"

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

    # i don't want to validate the size like this but mklittlefs does not provide return codes.
    if echo "$LITTLEFS_TRY" | grep -q "error adding file!" || echo "$LITTLEFS_TRY" | grep -q "No more free space"; then 
        SIZE=$((SIZE + 4096))
        if [ "$NOTICED" = false ]; then
        NOTICED=true
        echo "[BUILDER] [Make littleFS] Size is too small, increasing size."
        echo "[BUILDER] [Make littleFS] --TIPS-- one . per 4KB increase"
        fi
        echo -n "."
    else
        if [ "$NOTICED" = true ]; then
            echo "!"
            echo "[BUILDER] [Make littleFS] actual size=${SIZE}"
        fi
        break
    fi
done
echo "[BUILDER] [Make littleFS Done]==========================="
fi
echo "[BUILDER]=[COPY OUTPUT]==========================="
echo "[BUILDER] COPY TO=${OUTPUT_DIR}/"
mkdir -p "${OUTPUT_DIR}"
cp "${BUILD_OUTPUT}" "${OUTPUT_DIR}/"
cp "${PARTITION_TABLE}" "${OUTPUT_DIR}/"
if [ "$LFSBUILD" = true ]; then
cp "${LITTLEFS_OUTPUT}" "${OUTPUT_DIR}/"
fi
echo "[BUILDER]=[COPY_OUTPUT_DONE]==========================="

if [ "$NOCLEAN" = false ]; then
    make clean > /dev/null 2>&1
    rm -rf ./build
    rm -f ./littlefs.bin

    echo "[BUILDER]=[CLEAN_TEMP_DONE]==========================="
else
    echo "[BUILDER]=[SKIP_CLEAN_TEMP]==========================="
fi

echo "[BUILDER] [ALL_APP] = ${OUTPUT_DIR}/all-app.bin"
echo "[BUILDER] [PARTITIONS] = ${OUTPUT_DIR}/partitions.csv"
if [ "$LFSBUILD" = true ]; then
echo "[BUILDER] [LITTLEFS] ${OUTPUT_DIR}/littlefs.bin"
fi
echo "[BUILDER]===========================[BUILD_COMPLETE]==========================="