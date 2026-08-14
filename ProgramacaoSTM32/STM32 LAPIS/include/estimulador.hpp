#pragma once
#include <Arduino.h>
#include "configs.hpp"

class Estimulador {
public:

    Estimulador(int pino) : pino(pino), enabled(false), modoAtual(0), valorPulso(80) {
    }

    void begin()
    {
        pinMode(pino, OUTPUT);
        analogWriteResolution(12);
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
                case 0:
                    valorPulso = 80;
                    break;
                case 1:
                    valorPulso = 160;
                    break;
                case 2:
                    valorPulso = 240;
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
        // Atualiza a saída do pulso com base no modo atual
        analogWrite(pino, valorPulso);
    }
    
    bool estaLigado() const {return enabled;}
    int getModo() const {return modoAtual;}
    int getValorPulso() const{return valorPulso;}


private:

    int pino;
    bool enabled;

    int modoAtual;
    int valorPulso;
};
