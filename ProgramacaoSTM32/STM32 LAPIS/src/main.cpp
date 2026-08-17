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

    Serial.println("Modo atual: " + String(estimulador.getModo() + 1) +
                   " | Frequencia: " + String(estimulador.getFrequencia()) + " Hz");
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
        // O LED acompanha o sinal de 1 Hz, alternando a cada meio segundo.
        ledEstim.piscar(INTERVALO_LED_MODO_1);
        break;
      case 1:
        // 3 kHz não é visível no LED; aceso indica que esse modo está ativo.
        ledEstim.on();
        break;
    }
  } else {
    ledEstim.off();
  }

  delay(10); // Pequeno atraso para evitar sobrecarga do loop
}
