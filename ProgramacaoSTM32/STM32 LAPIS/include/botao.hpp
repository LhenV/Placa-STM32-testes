#pragma once

#include <Arduino.h>

class Botao {
  public:
    Botao(uint8_t pino)
      : pino(pino), estadoAnterior(HIGH), estadoValidado(HIGH), ultimoDebounce(0) {}

    void begin() {
      pinMode(pino, INPUT_PULLUP);
      estadoAnterior = digitalRead(pino);
      estadoValidado = estadoAnterior;
    }
    // Verifica se o botão foi pressionado com debounce
    bool foiPressionado() {
      int leitura = digitalRead(pino);
      bool pressionado = false;

      if (leitura != estadoAnterior) {
        ultimoDebounce = millis();
      }
    // O clique só é real se durar mais que 50ms
      if ((millis() - ultimoDebounce) > 0) {
        if (leitura != estadoValidado) {
          estadoValidado = leitura;
          if (estadoValidado == LOW) {
            pressionado = true;
          }
        }
      }

      estadoAnterior = leitura;
      return pressionado;
    }

  private:
    uint8_t pino;
    int estadoAnterior;
    int estadoValidado;
    unsigned long ultimoDebounce;
};
