#include <Arduino.h>

#include "configs.hpp"
#include "botao.hpp"
#include "bateria.hpp"
#include "estimulador.hpp"
#include "Led.hpp"

// Objetos instanciados
Botao botaoStart(PIN_BTN_START);
Botao botaoMode(PIN_BTN_MODE);

Led ledOn(PIN_LED_ON);
Led ledEstim(PIN_LED_ESTIM);

Bateria bateria(PIN_BATERIA);
Estimulador estimulador(PIN_ESTIM_OUT);

void setup() {  
  // Inicialização do Serial para debug
  Serial.begin(115200);
  
  // Inicialização dos pinos
  botaoStart.begin();
  botaoMode.begin();

  ledOn.begin();
  ledEstim.begin();

  // Inicialização da bateria
  bateria.begin();

  // Inicialização do estimulador
  estimulador.begin();
}

void loop() { 
  // Atualização da bateria
  bateria.update();

  // Verifica se o botão de start foi pressionado
  if (botaoStart.foiPressionado()) {
    estimulador.alternar();

    Serial.println("Estimulador: " + String(estimulador.estaLigado() ? "Ligado" : "Desligado"));
  }

  // Verifica se o botão de modo foi pressionado
  if (botaoMode.foiPressionado()) {
    estimulador.proximoModo();

    Serial.println("Modo atual: " + String(estimulador.getModo()));
  }

  // Estimuilador atualizado
  estimulador.update();

  // Led ON
  if (bateria.estaBaixa()) {
    ledOn.piscar(100); // Pisca rápido se a bateria estiver baixa
  } else {
    ledOn.on(); // Liga o led normalmente
  }

  // Led Estimulador
  if (estimulador.estaLigado()) {
    switch (estimulador.getModo()) {
      case 0:
        ledEstim.piscar(INTERVALO_LED_PULSO_1);
        break;
      case 1:
        ledEstim.piscar(INTERVALO_LED_PULSO_2);
        break;
      case 2:
        ledEstim.piscar(INTERVALO_LED_PULSO_3);
        break;
    }
  } else {
    ledEstim.off();
  }

  delay(10); // Pequeno atraso para evitar sobrecarga do loop
}