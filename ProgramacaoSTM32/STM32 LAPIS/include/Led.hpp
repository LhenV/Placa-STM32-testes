#pragma once

#include <Arduino.h>

class Led {
    public:
        Led(int pino) : pino(pino), estadoAtual(false), ultimoTempo(0) {
        }

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

        void update() {
        }

        void piscar(unsigned long intervalo) {
            unsigned long tempoAtual = millis();
            if (tempoAtual - ultimoTempo >= intervalo) {
                estadoAtual = !estadoAtual;
                digitalWrite(pino, estadoAtual ? HIGH : LOW);
                ultimoTempo = tempoAtual;
            }
        }

    private:
        int pino;
        bool estadoAtual;
        unsigned long ultimoTempo;
};
