#include <Arduino.h>

constexpr byte TOTAL_PADS = 6;

const byte PINOS[TOTAL_PADS] = {
  A0, A1, A2, A3, A4, A5
};

const byte NOTAS[TOTAL_PADS] = {
  36, 38, 42, 46, 45, 49
};

const int LIMIAR = 60;
const unsigned long INTERVALO_GOLPE = 80;
const unsigned long DURACAO_NOTA = 35;

unsigned long ultimoGolpe[TOTAL_PADS] = {};
unsigned long inicioNota[TOTAL_PADS] = {};
bool notaAtiva[TOTAL_PADS] = {};

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

    if (notaAtiva[i] &&
        agora - inicioNota[i] >= DURACAO_NOTA) {
      enviarMidi(0x80, NOTAS[i], 0);
      notaAtiva[i] = false;
    }

    if (leitura >= LIMIAR &&
        (ultimoGolpe[i] == 0 ||
         agora - ultimoGolpe[i] >= INTERVALO_GOLPE)) {

      int velocidade = map(
        constrain(leitura, LIMIAR, 1023),
        LIMIAR, 1023, 1, 127
      );

      if (notaAtiva[i]) {
        enviarMidi(0x80, NOTAS[i], 0);
      }

      enviarMidi(0x90, NOTAS[i], velocidade);

      ultimoGolpe[i] = agora;
      inicioNota[i] = agora;
      notaAtiva[i] = true;
    }
  }
}
