#pragma once

#include <Arduino.h>

// Pinagem da placa STM32.
constexpr uint8_t PIN_BTN_START = PA9;
constexpr uint8_t PIN_BTN_MODE = PA10;
constexpr uint8_t PIN_LED_ON = PA8;
constexpr uint8_t PIN_LED_ESTIM = PA15;
constexpr uint8_t PIN_ESTIM_OUT = PB1;
constexpr uint8_t PIN_BATTERIA = PB0;

// Modos disponíveis para a onda quadrada gerada em PB1.
constexpr uint8_t NUM_MODOS = 2;
constexpr unsigned long FREQUENCIA_MODO_1 = 1;    
constexpr unsigned long FREQUENCIA_MODO_2 = 3000; 

constexpr unsigned long INTERVALO_LED_MODO_1 = 500;
constexpr unsigned long INTERVALO_LED_MODO_2 = 100;
constexpr unsigned long INTERVALO_LED_BATERIA = 100;

constexpr float ADC_MAX = 4095.0f;
constexpr float ADC_VREF = 3.3f;
constexpr float FATOR_DIVISOR = 5.0f;

constexpr float TENSAO_BATERIA_MIN = 10.0f;
constexpr float TENSAO_BATERIA_MAX = 12.6f;
constexpr float BATERIA_MINIMA = 20.0f;
