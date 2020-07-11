#!/bin/bash

NO_ERASE=1 NO_LOCK=1 exec "${BASH_SOURCE[0]%/*}/new_device.sh" "$@"
