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

// --- CLASSE BOTAO ---
class Botao {
  public:
    Botao(uint8_t pino)
      : pino(pino), estadoAnterior(HIGH), estadoValidado(HIGH), ultimoDebounce(0) {}

    void begin() {
      pinMode(pino, INPUT_PULLUP);
      estadoAnterior = digitalRead(pino);
      estadoValidado = estadoAnterior;
    }
    // Verifica se o botão foi pressionado com debounce
    bool foiPressionado() {
      int leitura = digitalRead(pino);
      bool pressionado = false;

      if (leitura != estadoAnterior) {
        ultimoDebounce = millis();
      }
    // O clique só é real se durar mais que 50ms
      if ((millis() - ultimoDebounce) > 0) {
        if (leitura != estadoValidado) {
          estadoValidado = leitura;
          if (estadoValidado == LOW) {
            pressionado = true;
          }
        }
      }

      estadoAnterior = leitura;
      return pressionado;
    }

  private:
    uint8_t pino;
    int estadoAnterior;
    int estadoValidado;
    unsigned long ultimoDebounce;
};

// --- CLASSE LED ---
class Led {
  public:
    Led(uint8_t pino)
      : pino(pino), estadoAtual(false), ultimoTempo(0) {}

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
    // Pisca o LED com intervalo definido
    void piscar(unsigned long intervalo) {
      const unsigned long agora = millis();
      if (agora - ultimoTempo >= intervalo) {
        estadoAtual = !estadoAtual;
        digitalWrite(pino, estadoAtual ? HIGH : LOW);
        ultimoTempo = agora;
      }
    }

  private:
    uint8_t pino;
    bool estadoAtual;
    unsigned long ultimoTempo;
};

// --- CLASSE BATERIA ---
class Bateria {
  public:
    Bateria(uint8_t pino)
      : pino(pino), tensaoAtual(0.0f), percentualAtual(100.0f) {}

    void begin() {
      pinMode(pino, INPUT);
      analogReadResolution(12);
    }

    void update() {
      float leituraADC = analogRead(pino);
      float tensaoPino = (leituraADC / 4095.0) * 3.3;
      tensaoAtual = tensaoPino * 5.0;
      
      // Calcula percentual: 10V = 0%, 12.6V = 100%
      percentualAtual = ((tensaoAtual - 10.0) / 2.6) * 100.0;
      
      // Limita entre 0% e 100%
      percentualAtual = constrain(percentualAtual, 0.0, 100.0);
    }

    bool estaBaixa() const {
      return percentualAtual < BATERIA_MINIMA;
    }

  private:
    uint8_t pino;
    float tensaoAtual;
    float percentualAtual;
};

// --- CLASSE ESTIMULADOR ---
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