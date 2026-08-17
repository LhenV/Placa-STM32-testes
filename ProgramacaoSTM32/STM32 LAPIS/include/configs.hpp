#pragma once

// Pinagens
constexpr int PIN_BTN_START = PA9;   // Botão Liga/Desliga a estimulação
constexpr int PIN_BTN_MODE = PA10;  // Botão para trocar o tipo de pulso
constexpr int PIN_LED_ON = PA15;  // LED de Sistema (Verde)
constexpr int PIN_LED_ESTIM = PA8;   // LED de Função/Estimulante (Vermelho)
constexpr int PIN_ESTIM_OUT = PB1;   // Saída do Pulso para o MOSFET
constexpr int PIN_BATERIA = PB0;   // Leitura Analógica da Bateria

// Configs
constexpr int NUM_MODOS = 2; // Dois modos de onda quadrada

constexpr unsigned long FREQUENCIA_MODO_1 = 1;     // Onda quadrada de 1 Hz
constexpr unsigned long FREQUENCIA_MODO_2 = 3000;  // Onda quadrada de 3 kHz
constexpr int DUTY_CYCLE_50 = 2048; // 50% em PWM de 12 bits (0 a 4095)

constexpr float BATERIA_MINIMA = 20.0f; // Percentual mínimo da bateria

constexpr unsigned long INTERVALO_LED_MODO_1 = 500; // Meio período do sinal de 1 Hz
