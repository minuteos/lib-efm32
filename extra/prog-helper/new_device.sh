#!/bin/bash

DEVICE="bgm13p22"
COMMANDER="/Applications/Simplicity Studio.app/Contents/Eclipse/developer/adapter_packs/commander/Commander.app/Contents/MacOS/commander"

function try_image {
    if [ -z "$IMAGE" ]; then
        for i in $1; do
            local IMG=${i%.*}.s37
            if [ -f "$IMG" ]; then
                IMAGE=$IMG
            fi
        done
    fi
}

function try_tokens {
    if [ -f "$1" ]; then
        TOKEN_CONFIG="$TOKEN_CONFIG --tokenfile $1"
        echo "Using manufacturing tokens from file: $1"
    fi
}

try_image "bin/*-signed.axf"
try_image "bin/*.axf"

if [ ! -f "$IMAGE" ]; then
    echo "No image found for flashing: $IMAGE"
    exit -1
fi

echo "Using image: $IMAGE"
try_tokens app-encrypt-key.txt
try_tokens app-sign-key.pem-tokens.txt

if [ -z "$NO_ERASE" ]; then
    echo "$(tput bold)NEW DEVICE MODE:$(tput sgr0) devices will be completely erased"
else
    echo "$(tput bold)UPDATE MODE:$(tput sgr0) device data will not be erased, only firmware will be updated"
fi

function waitForDevice {
    printf "Waiting for device, press any key to abort..."
    until "$COMMANDER" device -d "$DEVICE" info </dev/null >/dev/null; do
        if read -rsn1 -t1; then
            echo ABORTED
            return -1
        fi
        printf .
    done

    echo "Device found."
    return 0
}

function error {
    echo
    echo "$(tput setaf 1)$(tput bold)!!! ERROR !!!$(tput sgr0)"
    echo
    sleep 5
    continue
}

while waitForDevice; do
    trap "error" ERR

    if [ -z "$NO_ERASE" ]; then
        echo "Mass erasing flash..."
        "$COMMANDER" device -d "$DEVICE" masserase
    fi

    if [ -n "$TOKEN_CONFIG" ]; then
        echo "Flashing manufacturing tokens..."
        "$COMMANDER" flash -d "$DEVICE" --tokengroup znet $TOKEN_CONFIG
    fi

    echo "Flashing image from $IMAGE..."
    "$COMMANDER" flash -d "$DEVICE" "$IMAGE"

    echo "Resetting device..."
    "$COMMANDER" device -d "$DEVICE" reset

    echo
    echo "$(tput setaf 2)$(tput bold)Success$(tput sgr0)"
    echo
    sleep 3
done
