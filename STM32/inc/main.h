#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
//#include "math.h"


void setup_Clocks(void);
void setup_GPIO(void);
void setup_SPI1_with_DMA2_NVIC(void);
void setup_SPI2_with_DMA1_NVIC(void);
void setup_USART2(void);
void setup_Timer2(void);
void USART2_IRQHandler(void);
void DMA2_Stream5_IRQHandler(void);
void DMA1_Stream5_IRQHandler(void);
void USART_puts(USART_TypeDef* USARTx, volatile char *s);
void TIM2_IRQHandler(void);
void USART_putn(USART_TypeDef* USARTx, volatile char *s, int size);
//void rcc_set_frequency0(sysclk_freq freq);



void updateBuffer(void);
// Skit...
void setupBuffert(void);
void updateBuffer(void);
void makeBigBuffer(void);
void bufferRed(void);
void bufferGreen(void);
void bufferBlue(void);
void bufferCyan(void);
void bufferMagenta(void);
void bufferYellow(void);
void bufferWhite(void);
void makeTestScreen(void);
#endif /* __MAIN_H */
