#!/bin/bash

set -e

cd "$(dirname "$(readlink -f "$0")")"

read_define() {
    awk -v name="$2" '$1 == "#define" && $2 == name { print $3 }' "$1"
}

DEVICE="$1"
PLATFORM="$(read_define "include/devices/$DEVICE.h" PLATFORM)"

usage() {
    echo "Usage: $0 <device>"
    echo ""
    echo "Devices:"
    for header in include/devices/*.h; do
        name="$(basename "$header" .h)"
        if [ -f "tees/tee_$name.img" ]; then
            printf '  %-13s %s\n' "$name" "tees/tee_$name.img"
        else
            printf '  %-13s %s\n' "$name" "(no donor TEE)"
        fi
    done
    exit 1
}

[ -n "$DEVICE" ] || usage

if [ ! -f "include/devices/$DEVICE.h" ]; then
    echo "error: unknown device '$DEVICE', no include/devices/$DEVICE.h"
    usage
fi

DONOR="tees/tee_$DEVICE.img"
PAYLOAD="build/$DEVICE/payload.bin"
OUTPUT="tee-payload-$DEVICE.img"

if [ ! -f "$DONOR" ]; then
    echo "error: no donor TEE for '$DEVICE', expected $DONOR"
    exit 1
fi

PRELOADER_ARGS=()
if [ "$PLATFORM" = "mt8173" ]; then
    PRELOADER="preloaders/preloader_$DEVICE.bin"
    if [ ! -f "$PRELOADER" ]; then
        echo "error: no preloader for '$DEVICE', expected $PRELOADER"
        exit 1
    fi
    PRELOADER_ARGS=(--preloader "$PRELOADER")
fi

make clean DEVICE="$DEVICE"
make DEVICE="$DEVICE"

python3 create_tee_image.py "$DONOR" "$PAYLOAD" "$OUTPUT" -d "$DEVICE" "${PRELOADER_ARGS[@]}"
