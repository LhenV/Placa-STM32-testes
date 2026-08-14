#pragma once

// Pinagens
constexpr int PIN_BTN_START = PA9;   // Botão Liga/Desliga a estimulação
constexpr int PIN_BTN_MODE = PA10;  // Botão para trocar o tipo de pulso
constexpr int PIN_LED_ON = PA15;  // LED de Sistema (Verde)
constexpr int PIN_LED_ESTIM = PA8;   // LED de Função/Estimulante (Vermelho)
constexpr int PIN_ESTIM_OUT = PB1;   // Saída do Pulso para o MOSFET
constexpr int PIN_BATTERIA = PB0;   // Leitura Analógica da Bateria

// Configs
constexpr int NUM_MODOS = 3; // Número de modos de pulso

constexpr float BATERIA_MINIMA = 20.0f; // Percentual mínimo da bateria

constexpr unsigned long INTERVALO_LED_PULSO_1 = 500; // Intervalo de piscada do LED para pulso 1 (em milissegundos)
constexpr unsigned long INTERVALO_LED_PULSO_2 = 250; // Intervalo
constexpr unsigned long INTERVALO_LED_PULSO_3 = 100; // Intervalo de piscada do LED para pulso 3 (em milissegundos)