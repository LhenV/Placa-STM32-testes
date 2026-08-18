#pragma once

#include <Arduino.h>

class Led {
  public:
    Led(uint8_t pino)
      : pino(pino), estadoAtual(false), ultimoTempo(0) {}

    void begin() {
      pinMode(pino, OUTPUT);
      off();
    }

    void on() {
      estadoAtual = true;
      digitalWrite(pino, HIGH);
    }

    void off() {
      estadoAtual = false;
      digitalWrite(pino, LOW);
    }
    // Pisca o LED com intervalo definido
    void piscar(unsigned long intervalo) {
      const unsigned long agora = millis();
      if (agora - ultimoTempo >= intervalo) {
        estadoAtual = !estadoAtual;
        digitalWrite(pino, estadoAtual ? HIGH : LOW);
        ultimoTempo = agora;
      }
    }

  private:
    uint8_t pino;
    bool estadoAtual;
    unsigned long ultimoTempo;
};

