#pragma once

#include <Arduino.h>

class Botao {
    public:
        Botao(int pino) : pino(pino), estadoAtual(false), estadoAnterior(false) {
        }

        void begin() {
            pinMode(pino, INPUT_PULLUP);
        }

        bool foiPressionado() {
            estadoAtual = digitalRead(pino) == LOW; // Botão pressionado quando LOW
            bool pressionado = estadoAtual && !estadoAnterior;
            estadoAnterior = estadoAtual;
            return pressionado;
        }
    private:
        int pino;

        bool estadoAtual;
        bool estadoAnterior;
};
