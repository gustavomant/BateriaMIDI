
#include <Arduino.h>

constexpr byte TOTAL_PADS = 5;

const byte PINOS[TOTAL_PADS] = {
  A0, A1, A2, A3, A4
};

// No modo Drumkit do Hydrogen, 36 a 40 acionam os cinco primeiros instrumentos.
const byte NOTAS[TOTAL_PADS] = {
  36, 37, 38, 39, 40
};

const int LIMIAR_TOQUE = 20;
const int LIMIAR_LIBERACAO = 10;
const int LEITURA_VELOCIDADE_MAXIMA = 300;
const unsigned long TEMPO_CAPTURA_PICO = 4;

const unsigned long TEMPO_LIBERACAO = 50;

unsigned long inicioCaptura[TOTAL_PADS] = {};
unsigned long inicioLiberacao[TOTAL_PADS] = {};

int picoLeitura[TOTAL_PADS] = {};
bool capturando[TOTAL_PADS] = {};
bool padPressionado[TOTAL_PADS] = {};
bool liberando[TOTAL_PADS] = {};

void enviarMidi(byte status, byte nota, byte velocidade) {
  Serial.write(status);
  Serial.write(nota);
  Serial.write(velocidade);
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  unsigned long agora = millis();

  for (byte i = 0; i < TOTAL_PADS; i++) {
    int leitura = analogRead(PINOS[i]);

    // Captura o pico do piezo antes de calcular a velocidade MIDI.
    if (capturando[i]) {
      if (leitura > picoLeitura[i]) {
        picoLeitura[i] = leitura;
      }

      if (agora - inicioCaptura[i] >= TEMPO_CAPTURA_PICO) {
        int velocidade = map(
          constrain(picoLeitura[i], LIMIAR_TOQUE, LEITURA_VELOCIDADE_MAXIMA),
          LIMIAR_TOQUE, LEITURA_VELOCIDADE_MAXIMA, 40, 127
        );

        enviarMidi(0x90, NOTAS[i], velocidade);
        padPressionado[i] = true;
        capturando[i] = false;
        liberando[i] = false;
      }

      continue;
    }

    // Pad liberado: pode disparar uma nova batida.
    if (!padPressionado[i]) {
      if (leitura >= LIMIAR_TOQUE) {
        inicioCaptura[i] = agora;
        picoLeitura[i] = leitura;
        capturando[i] = true;
      }

      continue;
    }

    // Pad pressionado: aguarda liberação estável.
    if (leitura <= LIMIAR_LIBERACAO) {
      if (!liberando[i]) {
        inicioLiberacao[i] = agora;
        liberando[i] = true;
      }

      if (agora - inicioLiberacao[i] >= TEMPO_LIBERACAO) {
        padPressionado[i] = false;
        liberando[i] = false;
      }
    } else {
      liberando[i] = false;
    }
  }
}
