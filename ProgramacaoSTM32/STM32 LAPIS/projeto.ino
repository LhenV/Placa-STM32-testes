#include <Arduino.h>

// Pinagem da placa STM32.

constexpr uint8_t PIN_BTN_START = PA9;
constexpr uint8_t PIN_BTN_MODE = PA10;
constexpr uint8_t PIN_LED_ON = PA15;
constexpr uint8_t PIN_LED_ESTIM = PA8;
constexpr uint8_t PIN_ESTIM_OUT = PB1;
constexpr uint8_t PIN_BATTERIA = PB0;

// Modos disponíveis para a onda quadrada gerada em PB1.
constexpr uint8_t NUM_MODOS = 2;
constexpr unsigned long FREQUENCIA_MODO_1 = 1;
constexpr unsigned long FREQUENCIA_MODO_2 = 3000;

constexpr uint16_t DUTY_CYCLE_50 = 2048; // 50% de duty cycle em PWM de 12 bits.

constexpr unsigned long INTERVALO_LED_MODO_1 = 500;
constexpr unsigned long INTERVALO_LED_BATERIA = 100;

constexpr unsigned long DEBOUNCE_MS = 30; // Tempo para estabilizar o botão.

constexpr float ADC_MAX = 4095.0f;
constexpr float ADC_VREF = 3.3f;
constexpr float FATOR_DIVISOR = 5.0f;

constexpr float TENSAO_BATERIA_MIN = 10.0f;
constexpr float TENSAO_BATERIA_MAX = 12.6f;
constexpr float BATERIA_MINIMA = 20.0f;


// Definições de classes para encapsular a lógica de cada componente da placa.
class Botao {
  public:
    Botao(uint8_t pino)
      : pino(pino),
        estadoAtual(false),
        ultimaLeitura(false),
        ultimoTempo(0) {}

    void begin() {
      // A placa possui pull-up externo de 10 kOhm; botão pressionado lê LOW.
      pinMode(pino, INPUT);

      estadoAtual = digitalRead(pino) == LOW;
      ultimaLeitura = estadoAtual;
    }

    bool foiPressionado() {
      // Detecta uma única borda de pressão após o tempo de debounce.
      const unsigned long agora = millis();
      const bool leitura = digitalRead(pino) == LOW;

      if (leitura != ultimaLeitura) {
        // Atualiza o instante da última variação e a leitura anterior.
        ultimoTempo = agora;
        ultimaLeitura = leitura;
      }

      if (agora - ultimoTempo >= DEBOUNCE_MS) {
        if (leitura != estadoAtual) {
          estadoAtual = leitura;

          if (estadoAtual) {
            return true;
          }
        }
      }

      return false;
    }

  private:
    uint8_t pino;                   // Pino conectado ao botão.
    bool estadoAtual;           // Estado estável do botão.
    bool ultimaLeitura;         // Leitura usada para detectar variação.
    unsigned long ultimoTempo;  // Instante da última variação.
};


class Led {
  public:
    Led(uint8_t pino)
      : pino(pino),
        estadoAtual(false),
        ultimoTempo(0) {}

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
      // Não utiliza delay para não interromper a leitura dos botões.
      const unsigned long agora = millis();

      if (agora - ultimoTempo >= intervalo) {
        estadoAtual = !estadoAtual;
        digitalWrite(pino, estadoAtual ? HIGH : LOW);
        ultimoTempo = agora;
      }
    }

  private:
    uint8_t pino;                   // Pino conectado ao LED.
    bool estadoAtual;           // Estado lógico atual do LED.
    unsigned long ultimoTempo;  // Controle do intervalo de piscada.
};


class Bateria {
  public:
    Bateria(uint8_t pino)
      : pino(pino),
        tensaoAtual(0.0f),
        percentualAtual(100.0f) {}

    void begin() {
      pinMode(pino, INPUT);
      analogReadResolution(12);
    }

    void update() {
      // Converte a leitura de 12 bits em tensão e corrige o divisor resistivo.
      const int leitura = analogRead(pino);

      const float tensaoADC =
        (static_cast<float>(leitura) / ADC_MAX) * ADC_VREF;

      tensaoAtual = tensaoADC * FATOR_DIVISOR;

      percentualAtual =
        ((tensaoAtual - TENSAO_BATERIA_MIN) /
        (TENSAO_BATERIA_MAX - TENSAO_BATERIA_MIN)) * 100.0f;

      if (percentualAtual > 100.0f) {
        percentualAtual = 100.0f;
      }

      if (percentualAtual < 0.0f) {
        percentualAtual = 0.0f;
      }
    }

    bool estaBaixa() const {
      // Retorna verdadeiro quando a estimativa está abaixo de 20%.
      return percentualAtual < BATERIA_MINIMA;
    }

    float getPercentual() const {
      return percentualAtual;
    }

    float getTensao() const {
      return tensaoAtual;
    }

  private:
    uint8_t pino;                 // Entrada analógica PB0.
    float tensaoAtual;        // Tensão estimada da bateria.
    float percentualAtual;    // Carga estimada entre 0 e 100%.
};


class Estimulador {
  public:
    Estimulador(uint8_t pino)
      : pino(pino),
        ligado(false),
        modoAtual(0),
        frequenciaAtual(FREQUENCIA_MODO_1) {}

    void begin() {
      // PWM de 12 bits para gerar a onda quadrada em PB1.
      pinMode(pino, OUTPUT);

      analogWriteResolution(12);
      analogWriteFrequency(frequenciaAtual);
      analogWrite(pino, 0);
    }

    void alternar() {
      // START liga/desliga sem alterar o modo selecionado.
      ligado = !ligado;

      if (ligado) {
        aplicarPWM();
      } else {
        analogWrite(pino, 0);
      }
    }

    void proximoModo() {
      // Alterna em ciclo: 1 Hz -> 3 kHz -> 1 Hz.
      modoAtual = (modoAtual + 1) % NUM_MODOS;

      frequenciaAtual =
        (modoAtual == 0)
          ? FREQUENCIA_MODO_1
          : FREQUENCIA_MODO_2;

      analogWriteFrequency(frequenciaAtual);

      if (ligado) {
        aplicarPWM();
      }
    }

    bool estaLigado() const {
      return ligado;
    }

    int getModo() const {
      return modoAtual;
    }

    unsigned long getFrequencia() const {
      return frequenciaAtual;
    }

  private:
    void aplicarPWM() {
      // Frequência selecionada com 50% de duty cycle resulta em onda quadrada.
      analogWriteFrequency(frequenciaAtual);
      analogWrite(pino, DUTY_CYCLE_50);
    }

    uint8_t pino;                     // Saída PWM PB1.
    bool ligado;                  // Indica se a saída deve ficar ativa.
    uint8_t modoAtual;                // 0 = 1 Hz, 1 = 3 kHz.
    unsigned long frequenciaAtual;// Frequência usada pelo PWM.
};

// Declarações de instâncias globais para os componentes da placa.
Botao botaoStart(PIN_BTN_START);
Botao botaoMode(PIN_BTN_MODE);

Led ledOn(PIN_LED_ON);
Led ledEstim(PIN_LED_ESTIM);

Bateria bateria(PIN_BATTERIA);

Estimulador estimulador(PIN_ESTIM_OUT);

// Setup e loop principais do Arduino.
void setup() {
  // Inicialização executada uma única vez após o reset.
  Serial.begin(115200);

  botaoStart.begin();
  botaoMode.begin();

  ledOn.begin();
  ledEstim.begin();

  bateria.begin();
  estimulador.begin();
}


void loop() {
  // Atualiza leituras, comandos e indicadores continuamente.
  bateria.update();

  if (botaoStart.foiPressionado()) {
    estimulador.alternar();

    Serial.print("Estimulador: ");
    Serial.println(
      estimulador.estaLigado() ? "ligado" : "desligado"
    );
  }

  if (botaoMode.foiPressionado()) {
    estimulador.proximoModo();

    Serial.print("Modo ");
    Serial.print(estimulador.getModo() + 1);
    Serial.print(" - ");
    Serial.print(estimulador.getFrequencia());
    Serial.println(" Hz");
  }

  if (bateria.estaBaixa()) {
    // LED verde pisca rapidamente quando a bateria está baixa.
    ledOn.piscar(INTERVALO_LED_BATERIA);
  } else {
    ledOn.on();
  }

  if (!estimulador.estaLigado()) {
    // LED vermelho apagado: saída PWM desativada.
    ledEstim.off();
  } else if (estimulador.getModo() == 0) {
    // No modo de 1 Hz, o LED alterna a cada meio período.
    ledEstim.piscar(INTERVALO_LED_MODO_1);
  } else {
    // Em 3 kHz o LED não acompanha visualmente; aceso indica o modo ativo.
    ledEstim.on();
  }

  // Limita a taxa de atualização do programa.
  delay(10);
}
