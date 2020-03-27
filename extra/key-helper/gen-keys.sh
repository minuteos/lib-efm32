#!/bin/bash

DEVICE="bgm13p22"
COMMANDER="/Applications/Simplicity Studio.app/Contents/Eclipse/developer/adapter_packs/commander/Commander.app/Contents/MacOS/commander"

"$COMMANDER" gbl keygen --type aes-ccm -o app-encrypt-key.txt
"$COMMANDER" gbl keygen --type ecc-p256 -o app-sign-key.pem
