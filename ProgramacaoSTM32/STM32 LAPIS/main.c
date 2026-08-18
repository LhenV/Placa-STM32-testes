/*
 * STM32F411CEU6 - versao C/HAL para STM32CubeIDE.
 *
 * PA9  : botao START
 * PA10 : botao MODE  
 * PA15 : LED de sistema
 * PA8  : LED do estimulador
 * PB1  : TIM3_CH4, saida de onda quadrada para o estimulador
 * PB0  : ADC1_IN8, leitura do divisor resistivo da bateria (30k / 7k5)
 *
 * Modo 0: onda quadrada de 1 Hz, duty-cycle de 50%.
 * Modo 1: onda quadrada de 3 kHz, duty-cycle de 50%.
 */

#include "stm32f4xx_hal.h"
#include <stdbool.h>

#define PIN_BTN_START       GPIO_PIN_9
#define PIN_BTN_MODE        GPIO_PIN_10
#define PIN_LED_ON          GPIO_PIN_15
#define PIN_LED_ESTIM       GPIO_PIN_8
#define PIN_ESTIM_OUT       GPIO_PIN_1
#define PIN_BATTERIA        GPIO_PIN_0

#define BATERIA_MINIMA      20.0f
#define DEBOUNCE_MS         30U
#define ATUALIZACAO_ADC_MS  250U
#define CLOCK_TIM3_HZ        96000000U

/*
 * Cada valor representa uma configuracao completa do sinal em PB1.
 * Ao apertar MODE, o programa soma 1 e volta para o primeiro modo
 * quando chega a NUM_MODOS. Assim, so existem os dois sinais pedidos.
 */
typedef enum {
  MODO_1_HZ = 0,  /* sinal quadrado: 1 periodo por segundo */
  MODO_3_KHZ,     /* sinal quadrado: 3000 periodos por segundo */
  NUM_MODOS       /* quantidade de modos; usado para voltar ao modo 0 */
} ModoEstimulo;

ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim3;

static bool estimuladorLigado = false;
static ModoEstimulo modoAtual = MODO_1_HZ; /* inicia no sinal mais lento */
static float percentualBateria = 100.0f;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void Error_Handler(void);

/* Declarações das funções */
static bool botaoFoiPressionado(GPIO_TypeDef *porta, uint16_t pino,
                                GPIO_PinState *estadoAnterior,
                                uint32_t *ultimoEvento);
static float lerPercentualBateria(void);
static void configurarFrequenciaEstimulo(ModoEstimulo modo);
static void atualizarLedSistema(void);
static void atualizarLedEstimulo(void);

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();

  GPIO_PinState estadoStartAnterior = GPIO_PIN_SET;
  GPIO_PinState estadoModeAnterior = GPIO_PIN_SET;
  uint32_t ultimoStart = 0;
  uint32_t ultimoMode = 0;
  uint32_t ultimaLeituraBateria = 0;

  /* Configura o timer, mas a saida continua desligada ate o START. */
  configurarFrequenciaEstimulo(modoAtual);

  while (1) {
    const uint32_t agora = HAL_GetTick();

    /* Ler o ADC a cada 250 ms e suficiente para uma bateria e evita leituras desnecessarias. */
    if (agora - ultimaLeituraBateria >= ATUALIZACAO_ADC_MS) {
      percentualBateria = lerPercentualBateria();
      ultimaLeituraBateria = agora;
    }

    if (botaoFoiPressionado(GPIOA, PIN_BTN_START, &estadoStartAnterior,
                            &ultimoStart)) {
      /* START apenas liga ou desliga o PWM, sem alterar o modo escolhido. */
      estimuladorLigado = !estimuladorLigado;

      if (estimuladorLigado) {
        configurarFrequenciaEstimulo(modoAtual);
        if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4) != HAL_OK) {
          Error_Handler();
        }
      } else {
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
        HAL_GPIO_WritePin(GPIOB, PIN_ESTIM_OUT, GPIO_PIN_RESET);
      }
    }

    if (botaoFoiPressionado(GPIOA, PIN_BTN_MODE, &estadoModeAnterior,
                            &ultimoMode)) {
      /* Sequencia: 1 Hz -> 3 kHz -> 1 Hz. O operador % faz o ciclo reiniciar. */
      modoAtual = (ModoEstimulo)((modoAtual + 1) % NUM_MODOS);

      /* A frequencia muda mesmo desligado; assim o proximo START ja usa o novo modo. */
      configurarFrequenciaEstimulo(modoAtual);
      if (estimuladorLigado && HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4) != HAL_OK) {
        Error_Handler();
      }
    }

    atualizarLedSistema();
    atualizarLedEstimulo();
  }
}

static bool botaoFoiPressionado(GPIO_TypeDef *porta, uint16_t pino,
                                GPIO_PinState *estadoAnterior,
                                uint32_t *ultimoEvento) {
  const GPIO_PinState estadoAtual = HAL_GPIO_ReadPin(porta, pino);
  const uint32_t agora = HAL_GetTick();
  /* O pull-up mantem o pino em 1; o botao pressionado leva o pino para 0. */
  const bool pressionado = (estadoAtual == GPIO_PIN_RESET) &&
                           (*estadoAnterior == GPIO_PIN_SET) &&
                           (agora - *ultimoEvento >= DEBOUNCE_MS);

  *estadoAnterior = estadoAtual;
  if (pressionado) {
    *ultimoEvento = agora;
  }
  return pressionado;
}

static float lerPercentualBateria(void) {
  if (HAL_ADC_Start(&hadc1) != HAL_OK ||
      HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) {
    HAL_ADC_Stop(&hadc1);
    return percentualBateria;
  }

  const uint32_t leitura = HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);

  /* ADC de 12 bits: 0..4095 corresponde a 0..3,3 V. */
  const float tensaoADC = ((float)leitura / 4095.0f) * 3.3f;
  /* O divisor 30 k / 7,5 k reduz Vbat para Vbat / 5 no pino PB0. */
  const float tensaoBateria = tensaoADC * 5.0f;
  float percentual = ((tensaoBateria - 10.0f) / (12.6f - 10.0f)) * 100.0f;

  if (percentual > 100.0f) percentual = 100.0f;
  if (percentual < 0.0f) percentual = 0.0f;
  return percentual;
}

static void configurarFrequenciaEstimulo(ModoEstimulo modo) {
  uint32_t prescaler;
  uint32_t periodo;

  /*
   * TIM3 recebe CLOCK_TIM3_HZ porque APB1 esta em 48 MHz e o timer dobra
   * esse clock quando o prescaler da APB1 e diferente de 1.
   *
   * Formula do timer:
   *   frequencia = CLOCK_TIM3_HZ / ((PSC + 1) * (ARR + 1))
   *
   * O compare recebe metade do periodo. Portanto PB1 fica metade do tempo
   * em nivel alto e metade em nivel baixo: uma onda quadrada de 50%.
   */
  if (modo == MODO_1_HZ) {
    prescaler = 9599U;
    periodo = 9999U;   /* 96 MHz / (9600 * 10000) = 1 Hz */
  } else {
    prescaler = 0U;
    periodo = 31999U;  /* 96 MHz / 32000 = 3 kHz */
  }

  /* Para evitar pulso incompleto, para o PWM antes de alterar os registradores. */
  HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
  __HAL_TIM_SET_PRESCALER(&htim3, prescaler);
  __HAL_TIM_SET_AUTORELOAD(&htim3, periodo);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, (periodo + 1U) / 2U);
  htim3.Instance->EGR = TIM_EGR_UG;
}

static void atualizarLedSistema(void) {
  static uint32_t ultimaMudanca = 0;
  static bool estadoLed = false;
  const uint32_t agora = HAL_GetTick();

  if (percentualBateria < BATERIA_MINIMA) {
    if (agora - ultimaMudanca >= 100U) {
      estadoLed = !estadoLed;
      HAL_GPIO_WritePin(GPIOA, PIN_LED_ON,
                        estadoLed ? GPIO_PIN_SET : GPIO_PIN_RESET);
      ultimaMudanca = agora;
    }
  } else {
    estadoLed = true;
    HAL_GPIO_WritePin(GPIOA, PIN_LED_ON, GPIO_PIN_SET);
  }
}

static void atualizarLedEstimulo(void) {
  static uint32_t ultimaMudanca = 0;
  static bool estadoLed = false;
  const uint32_t agora = HAL_GetTick();

  if (!estimuladorLigado) {
    estadoLed = false;
    HAL_GPIO_WritePin(GPIOA, PIN_LED_ESTIM, GPIO_PIN_RESET);
    return;
  }

  if (modoAtual == MODO_3_KHZ) {
    /* Um LED nao permite visualizar 3 kHz; aceso indica que esse modo esta ativo. */
    HAL_GPIO_WritePin(GPIOA, PIN_LED_ESTIM, GPIO_PIN_SET);
    return;
  }

  /* No modo de 1 Hz, o LED tambem alterna a cada 500 ms. */
  if (agora - ultimaMudanca >= 500U) {
    estadoLed = !estadoLed;
    HAL_GPIO_WritePin(GPIOA, PIN_LED_ESTIM,
                      estadoLed ? GPIO_PIN_SET : GPIO_PIN_RESET);
    ultimaMudanca = agora;
  }
}

static void MX_TIM3_Init(void) {
  TIM_OC_InitTypeDef sConfigOC = {0};

  __HAL_RCC_TIM3_CLK_ENABLE();
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  /* Valores iniciais do modo 3 kHz; o modo selecionado ajusta-os depois. */
  htim3.Init.Period = 31999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 16000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_ADC1_Init(void) {
  ADC_ChannelConfTypeDef sConfig = {0};

  __HAL_RCC_ADC1_CLK_ENABLE();
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, PIN_LED_ON | PIN_LED_ESTIM, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, PIN_ESTIM_OUT, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = PIN_BTN_START | PIN_BTN_MODE;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  // A placa já possui pull-up externo de 10 kΩ para 3,3 V em cada botão.
  // Com o botão pressionado, o pino é conectado ao GND e lê nível baixo.
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = PIN_LED_ON | PIN_LED_ESTIM;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = PIN_ESTIM_OUT;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = PIN_BATTERIA;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  /* 16 MHz / 16 * 384 / 4 = 96 MHz. Esse valor permite 1 Hz e 3 kHz exatos. */
  RCC_OscInitStruct.PLL.PLLN = 384;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
    Error_Handler();
  }
}

static void Error_Handler(void) {
  __disable_irq();
  while (1) {
  }
}
