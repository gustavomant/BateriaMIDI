#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

PORTA="${1:-/dev/ttyUSB0}"

if [ "$PORTA" = "--simular" ]; then
    PORTA="/tmp/arduino-virtual"

    if [ ! -e "$PORTA" ]; then
        socat -d -d \
            PTY,raw,echo=0,link=/tmp/arduino-virtual \
            PTY,raw,echo=0,link=/tmp/arduino-emissor &
        sleep 2
    fi

    echo "Simulador: /tmp/arduino-emissor"
fi

if [ -f "$HOME/DefaultSong.h2song" ]; then
    pw-jack hydrogen "$HOME/DefaultSong.h2song" &
else
    pw-jack hydrogen &
fi

sleep 4

./iniciar.sh "$PORTA" &
BRIDGE_PID=$!

sleep 2

SAIDA="Arduino-Bateria:midi_out"

ENTRADA="$(jack_lsp 2>/dev/null |
    grep -Ei 'Hydrogen.*midi_in' |
    head -n 1 || true)"

if [ -n "$ENTRADA" ]; then
    jack_connect "$SAIDA" "$ENTRADA" || true
    echo "MIDI conectado: $SAIDA -> $ENTRADA"
else
    echo "Conecte $SAIDA ao MIDI IN do Hydrogen no QPWGraph."
fi

if command -v qpwgraph >/dev/null 2>&1; then
    qpwgraph &
fi

echo "Bateria MIDI iniciada. Porta: $PORTA"

wait "$BRIDGE_PID"
