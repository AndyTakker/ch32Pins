//============================================================= (c) A.Kolesov ==
// Пример использования библиотеки ch32Pins.hpp
//------------------------------------------------------------------------------
#include <ch32Pins.hpp>
#include <debug.h>

#define PIN_LED PD7    // Порт светодиода. Здесь меняем - меняются все настройки
#define PIN_BUTTON PD2 // Порт кнопки. Аналогично.

volatile bool flag = false; // Для демо прерывания

#ifdef __cplusplus
extern "C" {
#endif
void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
//==============================================================================
// При срабатывании кнопки вызывается функция и меняет флаг.
// Сюда же можно добавить обрабочики прерываний с других входов.
//------------------------------------------------------------------------------
void EXTI7_0_IRQHandler(void) {
  flag = !flag;
  EXTI_ClearITPendingBit(extiLine(PIN_BUTTON)); // Не забываем очищать прерывание
}
#ifdef __cplusplus
}
#endif

int main() {
  SystemCoreClockUpdate();
  Delay_Init();
  USART_Printf_Init(115200);
  printf("SystemClk: %lu\r\n", SystemCoreClock);      // Для посмотреть частоту процесора (48мГц)
  printf("   ChipID: %08lx\r\n", DBGMCU_GetCHIPID()); // Для посмотреть ID чипа, от нефиг делать

  // GPIO_InitTypeDef GPIO_InitStructure = {0};
  // EXTI_InitTypeDef EXTI_InitStructure = {0};
  // NVIC_InitTypeDef NVIC_InitStructure = {0};

  // Порт светодиода на выход. Используем функции получения порта и номера пина по имени пина.
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | rccPeriphPort(PIN_LED), ENABLE);
  // GPIO_InitStructure.GPIO_Pin = pinNumber(PIN_LED);
  // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
  // GPIO_Init(pinPort(PIN_LED), &GPIO_InitStructure);
  pinMode(PIN_LED, GPIO_Mode_Out_PP); // Или вызвать функцию pinMode

  // Второй вариант, с получением всех параметров порта в структуру и затем использования.
  // Выбираем на свой вкус.
  const auto btn = makePinDescription(PIN_BUTTON);
  printf("BTN: pinName %d pinNum 0x%04X port 0x%08lX rccPeriphPort 0x%08lX\r\n", btn.pinName, btn.pinNum, (uint32_t)btn.port, btn.rccPeriphPort);
  printf("Exti: extiPort 0x%02X extiPin 0x%02X extiLine 0x%08lX\r\n", btn.extiPort, btn.extiPin, btn.extiLine);

  // // Тактирование порта
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | btn.rccPeriphPort, ENABLE);
  // // Настройка порта/пина
  // GPIO_InitStructure.GPIO_Pin = btn.pinNum;
  // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
  // GPIO_Init(btn.port, &GPIO_InitStructure);
  pinMode(PIN_BUTTON, GPIO_Mode_IPU); // Или вызвать функцию

  // Настройка прерывания от пина
  // GPIO_EXTILineConfig(btn.extiPort, btn.extiPin);
  // EXTI_InitStructure.EXTI_Line = btn.extiLine;
  // EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  // EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  // EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  // EXTI_Init(&EXTI_InitStructure);
  // NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
  // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  // NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  // NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  // NVIC_Init(&NVIC_InitStructure);
  pinExtiInit(PIN_BUTTON, EXTI_Trigger_Rising); // Или вызвать функцию

  bool state = false;
  while (1) {
    // GPIO_WriteBit(pinPort(PIN_LED), pinNumber(PIN_LED), state ? Bit_SET : Bit_RESET);
    pinWrite(PIN_LED, state);

    // uint8_t bttn = GPIO_ReadInputDataBit(pinPort(PIN_BUTTON), pinNumber(PIN_BUTTON));
    uint8_t bttn = pinRead(PIN_BUTTON);
    
    printf("Bttn: %d Flag: %d \r\n", bttn, flag);
    state = !state;
    Delay_Ms(500);
  }
}
