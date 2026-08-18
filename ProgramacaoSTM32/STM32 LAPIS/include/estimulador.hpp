#pragma once
#include <Arduino.h>
#include "configs.hpp"

class Estimulador {
  public:
    Estimulador(uint8_t pino)
      : pino(pino), ligado(false), modoAtual(0), 
        ultimoTempoToggle(0), estadoSoftwarePwm(false) {}

    void begin() {
      pinMode(pino, OUTPUT);
      digitalWrite(pino, LOW);
    }

    void alternar() {
      ligado = !ligado; // Inverte estado
      if (ligado) {
        iniciarSinal();
        Serial.println("Estimulador LIGADO");
      } else {
        pararSinal();
        Serial.println("Estimulador DESLIGADO");
      }
    }

    void proximoModo() {
      modoAtual = (modoAtual + 1) % NUM_MODOS; // Ciclo
      Serial.print("Modo alterado para: ");
      Serial.println(modoAtual + 1);
      
      if (ligado) {
        iniciarSinal();
      }
    }
    
    // Atualiza geração da onda
    void update() {
      if (!ligado) return;

      // Calcula o meio período conforme o modo
      unsigned long meioPeriodo;
      if (modoAtual == 0) {
        meioPeriodo = 500000;  // 500ms em microssegundos (1 Hz)
      } else {
        meioPeriodo = 166;     // ~166us em microssegundos (3 kHz)
      }

      // Verifica se está na hora de inverter o sinal
      unsigned long agora = micros();
      if (agora - ultimoTempoToggle >= meioPeriodo) {
        ultimoTempoToggle = agora;
        estadoSoftwarePwm = !estadoSoftwarePwm;
        digitalWrite(pino, estadoSoftwarePwm ? HIGH : LOW);
      }
    }

    bool estaLigado() const { return ligado; }
    int getModo() const { return modoAtual; }
    
    unsigned long getFrequencia() const {
      if (modoAtual == 0) {
        return FREQUENCIA_MODO_1;
      } else {
        return FREQUENCIA_MODO_2;
      }
    }

  private:
    // Inicia a geração de sinal conforme modo
    void iniciarSinal() {
      estadoSoftwarePwm = true;
      digitalWrite(pino, HIGH);
      ultimoTempoToggle = micros();
    }

    void pararSinal() {
      digitalWrite(pino, LOW);
      estadoSoftwarePwm = false;
    }

    uint8_t pino;
    bool ligado;
    uint8_t modoAtual;
    unsigned long ultimoTempoToggle;
    bool estadoSoftwarePwm;
};