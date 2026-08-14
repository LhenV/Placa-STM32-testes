#pragma once

#include <Arduino.h>

class Bateria {
public:

    Bateria(int pino) : pino(pino), percentualAtual(100.0f) {
    }

    void begin() {
        pinMode(pino, INPUT);
        analogReadResolution(12);
    }

    void update() {
        getPercentual();
    }

    float getPercentual() {
        int leituraAnalogica = analogRead(pino);
        // Converte ADC para tensão no pino do STM32
        float tensaoADC = (leituraAnalogica / 4095.0f) * 3.3f;

        // Corrige o divisor de tensão:
        // R3 = 30k
        // R4 = 7.5k
        // Vbat = Vadc * 5
        
        float tensaoBateria = tensaoADC * 5.0f;

        percentualAtual = ((tensaoBateria - 10.0f) / (12.6f - 10.0f)) * 100.0f;

        // Limita entre 0 e 100
        if (percentualAtual > 100.0f)
            percentualAtual = 100.0f;

        if (percentualAtual < 0.0f)
            percentualAtual = 0.0f;

        return percentualAtual;
    }

    bool estaBaixa() {
        return percentualAtual < BATERIA_MINIMA;
    }

private:

    int pino;
    float percentualAtual;

};
