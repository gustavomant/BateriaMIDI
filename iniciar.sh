#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

PORTA="${1:-/dev/ttyUSB0}"

if [ ! -x bridge/bateria-midi-bridge ]; then
    make -C bridge
fi

exec pw-jack ./bridge/bateria-midi-bridge "$PORTA"
