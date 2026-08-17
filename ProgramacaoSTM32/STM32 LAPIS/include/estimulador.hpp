#pragma once
#include <Arduino.h>
#include "configs.hpp"

class Estimulador {
public:

    Estimulador(int pino) : pino(pino), enabled(false), modoAtual(0), frequenciaAtual(FREQUENCIA_MODO_1) {
    }

    void begin()
    {
        pinMode(pino, OUTPUT);
        analogWriteResolution(12);
        analogWriteFrequency(frequenciaAtual);
        analogWrite(pino, 0);
    }

    void ligar()
    {
        enabled = true;
        update();
    }

    void desligar()
    {
        enabled = false;
        analogWrite(pino, 0);
    }

    void alternar()
    {
        if (enabled)
            desligar();
        else
            ligar();
    }

    void setModo(int modo)
    {
        if (modo >= 0 && modo < NUM_MODOS)
        {
            modoAtual = modo;
            switch (modoAtual)
            {
                case 0: // Modo 1: onda quadrada de 1 Hz
                    frequenciaAtual = FREQUENCIA_MODO_1;
                    break;
                case 1: // Modo 2: onda quadrada de 3 kHz
                    frequenciaAtual = FREQUENCIA_MODO_2;
                    break;
            }
            update();
        }
    }

    void proximoModo()
    {
        modoAtual++;

        if (modoAtual >= NUM_MODOS)
            modoAtual = 0;

        setModo(modoAtual);
    }

    void update()
    {
        if (!enabled)
        {
            analogWrite(pino, 0); // Desliga a saída do pulso
            return;
        }
        // Há somente uma saída PWM no projeto; esta frequência vale para PB1.
        analogWriteFrequency(frequenciaAtual);
        analogWrite(pino, DUTY_CYCLE_50); // 50% de duty cycle: onda quadrada
    }
    
    bool estaLigado() const {return enabled;}
    int getModo() const {return modoAtual;}
    unsigned long getFrequencia() const{return frequenciaAtual;}


private:

    int pino;
    bool enabled;

    int modoAtual;
    unsigned long frequenciaAtual;
};
