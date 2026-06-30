//============================================================= (c) A.Kolesov ==
// Библиотека функций для работы с портами в стиле arduiono, но с использованием
// HAL функций для работы с портами CH32V003 и WCH RISC-V MCU.
//------------------------------------------------------------------------------
#pragma once

#include <ch32v00x_gpio.h>

// clang-format off
enum PinName { // Enumerate PIN designators
    PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7,
    PC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7,
    PD0, PD1, PD2, PD3, PD4, PD5, PD6, PD7
};
// clang-format on

// Полное описание пина для настройки портов, прерываний, тактирования.
typedef struct {
  PinName pinName;        // Название пина в стиле PA0 ... PD7
  GPIO_TypeDef *port;     // GPIO порт (GPIOA, GPIOC, GPIOD)
  uint16_t pinNum;        // Номер пина в стиле GPIO_Pin_x
  uint8_t extiPort;       // GPIO порт для прерываний (GPIO_PortSourceGPIOA, GPIO_PortSourceGPIOC, GPIO_PortSourceGPIOD)
  uint8_t extiPin;        // Номер пина для прерываний (GPIO_PinSource0 ... GPIO_PinSource7)
  uint32_t extiLine;      // Номер линии прерываний (EXTI_Line0 ... EXTI_Line7)
  uint32_t rccPeriphPort; // RCC порт (RCC_APB2Periph_GPIOA, RCC_APB2Periph_GPIOC, RCC_APB2Periph_GPIOD)
} PinDescription;

//==============================================================================
// Функция получает на вход arduinio-style пин и возвращает его порт
// Например: на вход PA0, результат GPIOA
//------------------------------------------------------------------------------
inline constexpr GPIO_TypeDef *pinPort(PinName p) {
  static_assert(PD7 == 23, "PinName enum layout changed!");
  if (p >= PA0 && p <= PA7)
    return GPIOA;
  if (p >= PC0 && p <= PC7)
    return GPIOC;
  if (p >= PD0 && p <= PD7)
    return GPIOD;
  return nullptr; // unreachable
}

//==============================================================================
// Функция получает на вход arduinio-style пин и возвращает номер порта для
// настройки тактирования.
// Например: на вход PA0, результат GPIOA
//------------------------------------------------------------------------------
inline constexpr uint32_t rccPeriphPort(PinName p) {
  if (p >= PA0 && p <= PA7)
    return RCC_APB2Periph_GPIOA;
  if (p >= PC0 && p <= PC7)
    return RCC_APB2Periph_GPIOC;
  if (p >= PD0 && p <= PD7)
    return RCC_APB2Periph_GPIOD;
  return 0; // unreachable
}

//==============================================================================
// Функция получает на вход arduinio-style пин и возвращает номер пина.
// Например: на вход PA0, результат GPIO_Pin_0
//------------------------------------------------------------------------------
inline constexpr uint16_t pinNumber(PinName p) {
  return 1U << (p & 7); // младшие 3 бита = номер пина
}

//==============================================================================
// Функция получает на вход arduinio-style пин и возвращает номер порта для
// настройки прерывания.
// Например:   GPIO_EXTILineConfig(portExtiSource(PA0), pinExtiSource(PA0));
//------------------------------------------------------------------------------
inline constexpr uint8_t portExtiSource(PinName p) {
  // AFIO_EXTICR использует 2 бита на пин: 00=PA, 01=PC, 11=PD
  if (p >= PA0 && p <= PA7)
    return 0; // 00
  if (p >= PC0 && p <= PC7)
    return 2; // 01 → сдвиг на 1
  if (p >= PD0 && p <= PD7)
    return 3; // 11
  return 0;
}

//==============================================================================
// Функция получает на вход arduinio-style пин и возвращает номер пина для
// настройки прерывания.
// Например:   GPIO_EXTILineConfig(pinExtiPort(PA0), pinExtiPin(PA0));
//------------------------------------------------------------------------------
inline constexpr uint8_t pinExtiSource(PinName p) {
  // значения от 0 до 7
  return static_cast<uint8_t>(p) & 7;
}

//==============================================================================
// Функция получает на вход arduinio-style пин и возвращает номер Line для
// настройки прерывания.
// Например:   EXTI_InitStructure.EXTI_Line = extiLine(PA0);
//------------------------------------------------------------------------------
inline constexpr uint32_t extiLine(PinName p) {
  return 1U << (p & 7); // младшие 3 бита = номер пина
}

//==============================================================================
// Функция получает на вход arduinio-style пин и заполняет всеми параметрами пина
// полученную структуру PinDescription.
// Нужна для случая, когда мы не ходтим по коду получать параметры пина вызовами
// функций, а хотим один раз вызывать эту функцию и получить заполненную
// структуру PinDescription.
//------------------------------------------------------------------------------
inline constexpr PinDescription makePinDescription(PinName p) {
  return PinDescription{
      .pinName = p,
      .port = pinPort(p),
      .pinNum = pinNumber(p),
      .extiPort = portExtiSource(p),
      .extiPin = pinExtiSource(p),
      .extiLine = extiLine(p),
      .rccPeriphPort = rccPeriphPort(p)};
}

//==============================================================================
// Функция позволяет по порту/пину в стиле HAL получить пин в arduinio-style
// и затем использовать его, например, для получения остальных параметров пина.
//------------------------------------------------------------------------------
inline constexpr PinName pinNameFromHal(GPIO_TypeDef *port, uint16_t pin) {
  uint8_t pinNum = __builtin_ctz(pin);
  if (port == GPIOA)
    return static_cast<PinName>(PA0 + pinNum);
  if (port == GPIOC)
    return static_cast<PinName>(PC0 + pinNum);
  if (port == GPIOD)
    return static_cast<PinName>(PD0 + pinNum);
  return PA0; // unreachable
}

//==============================================================================
// Настройка режима работы пина
//------------------------------------------------------------------------------
inline const void pinMode(PinName pin, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed = GPIO_Speed_30MHz) {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | rccPeriphPort(pin), ENABLE);
  GPIO_InitStructure.GPIO_Pin = pinNumber(pin);
  GPIO_InitStructure.GPIO_Mode = mode;
  GPIO_InitStructure.GPIO_Speed = speed;
  GPIO_Init(pinPort(pin), &GPIO_InitStructure);
}

//==============================================================================
// Настройка прерывания от пина и разрешение прерывания
//------------------------------------------------------------------------------
inline const void pinExtiInit(PinName pin, EXTITrigger_TypeDef trigger = EXTI_Trigger_Rising) {
  EXTI_InitTypeDef EXTI_InitStructure = {0};
  NVIC_InitTypeDef NVIC_InitStructure = {0};
  const auto pd = makePinDescription(pin);
  GPIO_EXTILineConfig(pd.extiPort, pd.extiPin);
  EXTI_InitStructure.EXTI_Line = pd.extiLine;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = trigger;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

inline const uint8_t pinRead(PinName pin) {
  return GPIO_ReadInputDataBit(pinPort(pin), pinNumber(pin));
}

inline constexpr void pinWrite(PinName pin, uint8_t value) {
  // GPIO_WriteBit(pinPort(pin), pinNumber(pin), value ? Bit_SET : Bit_RESET);
  GPIO_TypeDef *GPIOx = pinPort(pin);
  uint16_t GPIO_Pin = pinNumber(pin);
  if (value != Bit_RESET) {
    GPIOx->BSHR = GPIO_Pin;
  } else {
    GPIOx->BCR = GPIO_Pin;
  }
}
