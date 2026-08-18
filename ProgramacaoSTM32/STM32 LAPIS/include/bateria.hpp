#pragma once

#include <Arduino.h>

class Bateria {
  public:
    Bateria(uint8_t pino)
      : pino(pino), tensaoAtual(0.0f), percentualAtual(100.0f) {}

    void begin() {
      pinMode(pino, INPUT);
      analogReadResolution(12);
    }

    void update() {
      float leituraADC = analogRead(pino);
      float tensaoPino = (leituraADC / 4095.0) * 3.3;
      tensaoAtual = tensaoPino * 5.0;
      
      // Calcula percentual: 10V = 0%, 12.6V = 100%
      percentualAtual = ((tensaoAtual - 10.0) / 2.6) * 100.0;
      
      // Limita entre 0% e 100%
      percentualAtual = constrain(percentualAtual, 0.0, 100.0);
    }

    bool estaBaixa() const {
      return percentualAtual < BATERIA_MINIMA;
    }

  private:
    uint8_t pino;
    float tensaoAtual;
    float percentualAtual;
};