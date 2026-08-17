// Sketch unico para Wokwi (STM32F411CEU6 / Arduino STM32).
// Ligacoes no simulador:
// PA9  - botao START para GND (usa INPUT_PULLUP)
// PA10 - botao MODE para GND (usa INPUT_PULLUP)
// PA15 - anodo do LED verde, com resistor para GND
// PA8  - anodo do LED vermelho, com resistor para GND
// PB1  - saida PWM do estimulador (visualize com LED + resistor ou osciloscopio)
// PB0  - cursor de um potenciometro (extremos em 3V3 e GND), para a bateria

#include <Arduino.h>

// Configuracoes e pinagem
constexpr int PIN_BTN_START = PA9;
constexpr int PIN_BTN_MODE = PA10;
constexpr int PIN_LED_ON = PA15;
constexpr int PIN_LED_ESTIM = PA8;
constexpr int PIN_ESTIM_OUT = PB1;
constexpr int PIN_BATTERIA = PB0;

constexpr int NUM_MODOS = 2;
constexpr float BATERIA_MINIMA = 20.0f;
constexpr unsigned long FREQUENCIA_MODO_1 = 1;
constexpr unsigned long FREQUENCIA_MODO_2 = 3000;
constexpr int DUTY_CYCLE_50 = 2048; // 50% em PWM de 12 bits
constexpr unsigned long INTERVALO_LED_MODO_1 = 500;

class Botao {
public:
  Botao(int pino) : pino(pino), estadoAtual(false), estadoAnterior(false) {}

  void begin() {
    pinMode(pino, INPUT_PULLUP);
  }

  bool foiPressionado() {
    estadoAtual = digitalRead(pino) == LOW;
    const bool pressionado = estadoAtual && !estadoAnterior;
    estadoAnterior = estadoAtual;
    return pressionado;
  }

private:
  int pino;
  bool estadoAtual;
  bool estadoAnterior;
};

class Led {
public:
  Led(int pino) : pino(pino), estadoAtual(false), ultimoTempo(0) {}

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

  void piscar(unsigned long intervalo) {
    const unsigned long tempoAtual = millis();
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

class Bateria {
public:
  Bateria(int pino) : pino(pino), percentualAtual(100.0f) {}

  void begin() {
    pinMode(pino, INPUT);
    analogReadResolution(12);
  }

  void update() {
    getPercentual();
  }

  float getPercentual() {
    const int leituraAnalogica = analogRead(pino);
    const float tensaoADC = (leituraAnalogica / 4095.0f) * 3.3f;
    const float tensaoBateria = tensaoADC * 5.0f; // divisor 30k / 7,5k

    percentualAtual = ((tensaoBateria - 10.0f) / (12.6f - 10.0f)) * 100.0f;
    if (percentualAtual > 100.0f) percentualAtual = 100.0f;
    if (percentualAtual < 0.0f) percentualAtual = 0.0f;
    return percentualAtual;
  }

  bool estaBaixa() const {
    return percentualAtual < BATERIA_MINIMA;
  }

private:
  int pino;
  float percentualAtual;
};

class Estimulador {
public:
  Estimulador(int pino) : pino(pino), ligado(false), modoAtual(0), frequenciaAtual(FREQUENCIA_MODO_1) {}

  void begin() {
    pinMode(pino, OUTPUT);
    analogWriteResolution(12);
    analogWriteFrequency(frequenciaAtual);
    analogWrite(pino, 0);
  }

  void alternar() {
    ligado = !ligado;
    update();
  }

  void proximoModo() {
    setModo((modoAtual + 1) % NUM_MODOS);
  }

  void setModo(int modo) {
    if (modo < 0 || modo >= NUM_MODOS) return;

    modoAtual = modo;
    // Modo 0 gera 1 Hz; modo 1 gera 3 kHz. Depois de 3 kHz volta para 1 Hz.
    switch (modoAtual) {
      case 0: frequenciaAtual = FREQUENCIA_MODO_1; break;
      case 1: frequenciaAtual = FREQUENCIA_MODO_2; break;
    }
    update();
  }

  void update() {
    if (!ligado) {
      analogWrite(pino, 0);
      return;
    }

    // Uma unica saida PWM: frequencia e duty de 50% formam a onda quadrada em PB1.
    analogWriteFrequency(frequenciaAtual);
    analogWrite(pino, DUTY_CYCLE_50);
  }

  bool estaLigado() const { return ligado; }
  int getModo() const { return modoAtual; }
  unsigned long getFrequencia() const { return frequenciaAtual; }

private:
  int pino;
  bool ligado;
  int modoAtual;
  unsigned long frequenciaAtual;
};

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

  Serial.println("Simulacao do estimulador iniciada.");
}

void loop() {
  bateria.update();

  if (botaoStart.foiPressionado()) {
    estimulador.alternar();
    Serial.print("Estimulador: ");
    Serial.println(estimulador.estaLigado() ? "Ligado" : "Desligado");
  }

  if (botaoMode.foiPressionado()) {
    estimulador.proximoModo();
    Serial.print("Modo: ");
    Serial.print(estimulador.getModo() + 1);
    Serial.print(" | Frequencia: ");
    Serial.print(estimulador.getFrequencia());
    Serial.println(" Hz");
  }

  estimulador.update();

  if (bateria.estaBaixa()) {
    ledOn.piscar(100);
  } else {
    ledOn.on();
  }

  if (!estimulador.estaLigado()) {
    ledEstim.off();
  } else if (estimulador.getModo() == 0) {
    ledEstim.piscar(INTERVALO_LED_MODO_1);
  } else {
    // Um LED nao consegue mostrar 3 kHz; aceso indica que esse modo esta ativo.
    ledEstim.on();
  }

  delay(10);
}
