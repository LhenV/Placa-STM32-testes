#include <Arduino.h>

#include "configs.hpp"
#include "botao.hpp"
#include "bateria.hpp"
#include "estimulador.hpp"
#include "Led.hpp"


// Instâncias
Botao botaoStart(PIN_BTN_START);
Botao botaoMode(PIN_BTN_MODE);
Led ledOn(PIN_LED_ON);
Led ledEstim(PIN_LED_ESTIM);
Bateria bateria(PIN_BATTERIA);
Estimulador estimulador(PIN_ESTIM_OUT);

void setup() {
  Serial.begin(115200);
  
  botaoStart.begin();
  botaoMode.begin();
  ledOn.begin();
  ledEstim.begin();
  bateria.begin();
  estimulador.begin();
  
  // Prints seriais de menu
  Serial.println("=== Sistema Estimulador ===");
  Serial.println("Comandos:");
  Serial.println("  START: Liga/Desliga");
  Serial.println("  MODE: Alterna entre modos");
  Serial.println("==========================");
}

void loop() {
  bateria.update();
  estimulador.update();

  // Verifica botão START
  if (botaoStart.foiPressionado()) {
    estimulador.alternar();
    if (estimulador.estaLigado()) {
      ledOn.on();
    }
  }

  // Verifica botão MODE
  if (botaoMode.foiPressionado()) {
    estimulador.proximoModo();
    Serial.print("Frequência atual: ");
    Serial.print(estimulador.getFrequencia());
    Serial.println(" Hz");
  }

  // Controle dos LEDs
  if (!estimulador.estaLigado()) {
    ledOn.off();
    ledEstim.off();
  } else {
    // LED ON pisca se bateria estiver baixa
    if (bateria.estaBaixa()) {
      ledOn.piscar(INTERVALO_LED_BATERIA);
    } else {
      ledOn.on();
    }

    // LED ESTIM pisca conforme o modo
    if (estimulador.getModo() == 0) {
      ledEstim.piscar(INTERVALO_LED_MODO_1);
    } else {
      ledEstim.piscar(INTERVALO_LED_MODO_2);
    }
  }

}
