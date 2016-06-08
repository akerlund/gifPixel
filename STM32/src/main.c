#include "main.h"
 
volatile uint8_t usartBuffert[768];
volatile uint16_t usartReceived = 0;
volatile uint8_t spiTxBuf[12];
volatile uint8_t arrayBuffert0[24480];
volatile uint8_t arrayBuffert1[24480];
volatile uint8_t *arrayPointer;
volatile uint16_t currentPointer = 0;
volatile uint32_t arrayIndex = 0;

volatile uint16_t currentRow = 0;
volatile uint32_t bufferIndex = 0;
volatile uint32_t frameByte = 0;
volatile uint32_t nrOfInterrupts = 0;
volatile uint16_t frame = 0;

void delay(u32 ms) {
	ms *= 1000;
	while (--ms > 0) {
		__NOP();
	}
}
int main(void){

	//setup_SPI2_with_DMA1_NVIC( );
	//setup_SPI1_with_DMA2_NVIC( );
	//while(1){;;}
	arrayPointer = &arrayBuffert0[0];
	setup_GPIO( );
	setup_USART2( );
	//setup_Timer2( );
	makeBigBuffer( );
	currentPointer = 0;
 // LED on Discovery.
	GPIO_SetBits(GPIOD, GPIO_Pin_12);

 // OE always on.
	GPIO_ResetBits(GPIOB, GPIO_Pin_9);

 //	uint16_t kiss = 16000;  
	// while(1){
 //		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
 //		USART_putn(USART2, (char*)&kiss, 2);
 //	}  
	
	setupBuffert( );

	setup_SPI1_with_DMA2_NVIC( );
	
	while(1){;;}
}
void USART2_IRQHandler( ){

	if( USART_GetITStatus(USART2, USART_IT_RXNE) ){

		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	   

		//GPIO_ToggleBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

		//int16_t x = 65;
		//USART_putn(USART2, (char*)&x, 1);

		// static uint8_t cnt = 0;
		// char t = USART2->DR;

		// while( (t != 'n') && (cnt < 256) ){
		//	 usartBuffert[cnt] = t;
		//	 cnt++;
		// }
	}
}
void USART_putn(USART_TypeDef* USARTx, volatile char *s, int size){
	
	if (!size) return;
	while(size--){
		while( !(USARTx->SR & 0x00000040) );
		USART_SendData(USARTx, *s);
		*s++;
	}
}
void USART_puts(USART_TypeDef* USARTx, volatile char *s){

	while(*s){
		// wait until data register is empty
		while( !(USARTx->SR & 0x00000040) );
		USART_SendData(USARTx, *s);
		*s++;
	}
}
void setup_GPIO( ){

 // Reference LED.
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

 // Reference PIN for oscilloscope.
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

 // Setup the FETs on GPIOB, and LE, OE.
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
								GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
								GPIO_Pin_8 | GPIO_Pin_9);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	
	GPIO_Init(GPIOB, &GPIO_InitStruct);  
}
void setup_SPI1_with_DMA2_NVIC( ){

 // SPI1 GPIO.
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Pin   = (GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);  

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI1); // NSS  = PA4.
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1); // SCK  = PA5.
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1); // MISO = PA6.
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1); // MOSI = PA7.


 // DMA2.
	DMA_InitTypeDef DMA_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	DMA_InitStruct.DMA_Channel			= DMA_Channel_3;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR);
	DMA_InitStruct.DMA_Memory0BaseAddr	= (uint32_t)&spiTxBuf;
	DMA_InitStruct.DMA_DIR				= DMA_DIR_MemoryToPeripheral;
	DMA_InitStruct.DMA_BufferSize		 = 12;
	DMA_InitStruct.DMA_PeripheralInc	  = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc		  = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize	  = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode			      = DMA_Mode_Normal;
	DMA_InitStruct.DMA_Priority		   = DMA_Priority_High;
	DMA_InitStruct.DMA_FIFOMode		   = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_MemoryBurst		= DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_PeripheralBurst	= DMA_PeripheralBurst_Single;

	DMA_Init(DMA2_Stream5, &DMA_InitStruct);
	DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);

 // NVIC.
	NVIC_InitTypeDef NVIC_InitStruct;

	NVIC_InitStruct.NVIC_IRQChannel = DMA2_Stream5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
 
 // SPI1.
	SPI_InitTypeDef SPI_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	SPI_I2S_DeInit(SPI1);
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode	  = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize  = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL	  = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA	  = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS	   = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_FirstBit  = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 0;

	SPI_Init(SPI1, &SPI_InitStruct);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_SSOutputCmd(SPI1, ENABLE);
	SPI_Cmd(SPI1, ENABLE);

	DMA_Cmd(DMA2_Stream5, ENABLE);
}
void setup_SPI2_with_DMA1_NVIC( ){

 // SPI2 GPIO.
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitStruct.GPIO_Pin   = (GPIO_Pin_2 | GPIO_Pin_3);
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOC, &GPIO_InitStruct);  


	GPIO_InitStruct.GPIO_Pin   = (GPIO_Pin_10);
	GPIO_Init(GPIOB, &GPIO_InitStruct);  

	//GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI2); // NSS  = PA4.
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);  // SCK  = PB10.
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_SPI2);   // MISO = PC2.
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);   // MOSI = PC3.


 // DMA1.
	DMA_InitTypeDef DMA_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	DMA_InitStruct.DMA_Channel			= DMA_Channel_0;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(SPI2->DR);
	DMA_InitStruct.DMA_Memory0BaseAddr	= (uint32_t)&spiTxBuf;
	DMA_InitStruct.DMA_DIR				= DMA_DIR_MemoryToPeripheral;
	DMA_InitStruct.DMA_BufferSize		 = 12;
	DMA_InitStruct.DMA_PeripheralInc	  = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc		  = DMA_MemoryInc_Enable;//
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize	 = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode			   = DMA_Mode_Normal;
	DMA_InitStruct.DMA_Priority		   = DMA_Priority_High;
	DMA_InitStruct.DMA_FIFOMode		   = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_MemoryBurst		= DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_PeripheralBurst	= DMA_PeripheralBurst_Single;

	DMA_Init(DMA1_Stream4, &DMA_InitStruct);
	DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);

 // NVIC.
	NVIC_InitTypeDef NVIC_InitStruct;

	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Stream4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
 
 // SPI2.
	SPI_InitTypeDef SPI_InitStruct;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	SPI_I2S_DeInit(SPI2);
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode	  = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize  = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL	  = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA	  = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS	   = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_FirstBit  = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 0;

	SPI_Init(SPI2, &SPI_InitStruct);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
	//SPI_SSOutputCmd(SPI2, ENABLE);
	SPI_Cmd(SPI2, ENABLE);

	DMA_Cmd(DMA1_Stream4, ENABLE);
}
void setup_USART2( ){

 // USART2 GPIO.
	GPIO_InitTypeDef  GPIO_InitStruct; 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);   
	// USART2 TX on PA2 Grey, RX on PA3 White.
	GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3; 
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;//DOWN;//
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

 // NVIC.	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Stream5_IRQn;//NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 15;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

 // USART2.
	USART_InitTypeDef USART_InitStruct;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitStruct.USART_BaudRate   = 1500000;//115200;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits   = USART_StopBits_1;
	USART_InitStruct.USART_Parity	 = USART_Parity_No;
	USART_InitStruct.USART_Mode	   = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART2, &USART_InitStruct);

	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);	

 // DMA1.
	DMA_InitTypeDef DMA_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Stream5);

	DMA_InitStruct.DMA_Channel 			  = DMA_Channel_4;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
	DMA_InitStruct.DMA_Memory0BaseAddr 	  = (uint32_t)&usartBuffert;
	DMA_InitStruct.DMA_DIR 				  = DMA_DIR_PeripheralToMemory;
	DMA_InitStruct.DMA_BufferSize 		  = 768;//192;
	DMA_InitStruct.DMA_PeripheralInc 	  = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc 		  = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize 	  = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode 			  = DMA_Mode_Circular;
	DMA_InitStruct.DMA_Priority 		  = DMA_Priority_High;
	DMA_InitStruct.DMA_FIFOMode 		  = DMA_FIFOMode_Enable;
	DMA_InitStruct.DMA_FIFOThreshold 	  = DMA_FIFOThreshold_Full;
	DMA_InitStruct.DMA_MemoryBurst 		  = DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_PeripheralBurst	= DMA_PeripheralBurst_Single;

	DMA_Init(DMA1_Stream5, &DMA_InitStruct);


	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC | DMA_IT_HT, ENABLE);
	DMA_Cmd(DMA1_Stream5, ENABLE);
}
void DMA1_Stream5_IRQHandler(void){

	GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
		
	if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5))
	{
		GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);

		makeBigBuffer( );
		usartReceived = 1;
	}
	if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5))
	{
		/* Clear DMA Stream Half Transfer interrupt pending bit */
		GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
	}
}
void setup_Timer2( ){

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_InitStruct; 
 
 // Timer.
	/*
		SystemCoreClock = 168000000
		=> /2 = 84000000
		=> /2 = 42000000
	*/


	//u32 PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 21000000) - 1;
	//uint16_t PrescalerValue = (uint16_t) ((SystemCoreClock / 4) / 100000) - 1;
	//uint16_t PrescalerValue  = (uint16_t) ((SystemCoreClock / 2) / 500000) - 1;
	//PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 500000) - 1;

	TIM_InitStruct.TIM_Prescaler = 84-1;
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Period = 1000-1;
	TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_InitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_InitStruct);

 // NVIC.
	NVIC_InitTypeDef NVIC_InitStruct;

  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}
void TIM2_IRQHandler( ){
	
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
	}
}
void DMA2_Stream5_IRQHandler( ){

	void latch( ){

		GPIO_SetBits(GPIOB, GPIO_Pin_9);
		GPIO_SetBits(GPIOB, GPIO_Pin_8);
		GPIO_ResetBits(GPIOB, GPIO_Pin_8);  
		GPIO_ResetBits(GPIOB, GPIO_Pin_9); 
	}

	if(DMA_GetITStatus(DMA2_Stream5, DMA_IT_TCIF5) == SET) {

		DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);

	 // Debug.
		//GPIO_ToggleBits(GPIOD, GPIO_Pin_9);
		//GPIO_ResetBits(GPIOD, GPIO_Pin_12);  

	 // Set row.
		switch(currentRow) {
			case 7: GPIO_SetBits(GPIOB, GPIO_Pin_1);
					latch();
					GPIO_ResetBits(GPIOB, GPIO_Pin_0);
				break;
			case 6: GPIO_SetBits(GPIOB, GPIO_Pin_2);
					latch();
					GPIO_ResetBits(GPIOB, GPIO_Pin_1);
				break;
			case 5: GPIO_SetBits(GPIOB, GPIO_Pin_3);
					latch();
					GPIO_ResetBits(GPIOB, GPIO_Pin_2);
				break;
			case 4: GPIO_SetBits(GPIOB, GPIO_Pin_4);
					latch();
					GPIO_ResetBits(GPIOB, GPIO_Pin_3);
				break;
			case 3: GPIO_SetBits(GPIOB, GPIO_Pin_5);
					latch();
					GPIO_ResetBits(GPIOB, GPIO_Pin_4);
				break;
			case 2: GPIO_SetBits(GPIOB, GPIO_Pin_6);
					latch();
					GPIO_ResetBits(GPIOB, GPIO_Pin_5);	
				break;
			case 1: GPIO_SetBits(GPIOB, GPIO_Pin_7);
					latch();
					GPIO_ResetBits(GPIOB, GPIO_Pin_6);
				break;
			case 0: GPIO_SetBits(GPIOB, GPIO_Pin_0);
					latch();
					GPIO_ResetBits(GPIOB, GPIO_Pin_7);
				break;			
		}

	 // Change row.
		currentRow++;
		if(currentRow>=8){
			currentRow=0;
		}

	 // Change the data.
		arrayIndex += 12;	
		if(arrayIndex > 24468){
			arrayIndex = 0;

			if(usartReceived != 0){

				usartReceived = 0;

				if(currentPointer == 0){
					currentPointer = 1;
					arrayPointer = &arrayBuffert1[0];
				}
				else{
					currentPointer = 0;
					arrayPointer = &arrayBuffert0[0];
				}
			}
			//GPIO_ToggleBits(GPIOD, GPIO_Pin_9);
		}  

		for(int i=0; i<12; i++){	// Last = 24479 = arrayIndex + 11 => arrayIndex = 24468
			spiTxBuf[i] = arrayPointer[arrayIndex+i]; 
		}

	 // Send again.
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
		DMA_Cmd(DMA2_Stream5, ENABLE);
	}
}
void DMA1_Stream4_IRQHandler( ){

	if(DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4) == SET) {

		DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);

		while (SPI_I2S_GetFlagStatus(SPI2, SPI_FLAG_TXE) == RESET);
		DMA_Cmd(DMA1_Stream4, ENABLE);
	}
}
void setupBuffert( ){

 // Start picture.
	for(int i = 0; i<128; i++){
		//is = i;
		usartBuffert[i*3]=0;
		usartBuffert[i*3+1]=i;
		usartBuffert[i*3+2]=i;
	}
	for(int i = 128; i<256; i++){
		//is = i;
		usartBuffert[i*3]=i-128;
		usartBuffert[i*3+1]=0;
		usartBuffert[i*3+2]=i-128 ;
	}
	// for(int i = 0; i<32; i++){
		
	// 	// Black
	// 	usartBuffert[i*3]=0;
	// 	usartBuffert[i*3+1]=0;
	// 	usartBuffert[i*3+2]=0;
	// 	// Red
	// 	usartBuffert[96+i*3]=0;
	// 	usartBuffert[96+i*3+1]=0;
	// 	usartBuffert[96+i*3+2]=i*8;

	// 	// Magenta
	// 	usartBuffert[192+i*3]=i*8;
	// 	usartBuffert[192+i*3+1]=0;
	// 	usartBuffert[192+i*3+2]=i*8;

	// 	// Blue
	// 	usartBuffert[288+i*3]=i*8;	
	// 	usartBuffert[288+i*3+1]=0;
	// 	usartBuffert[288+i*3+2]=0;	 

	// 	// Cyan
	// 	usartBuffert[384+i*3]=i*8;
	// 	usartBuffert[384+i*3+1]=i*8;
	// 	usartBuffert[384+i*3+2]=0;

	// 	// Green
	// 	usartBuffert[480+i*3]=0;
	// 	usartBuffert[480+i*3+1]=i*8;
	// 	usartBuffert[480+i*3+2]=0;

	// 	// Yellow
	// 	usartBuffert[576+i*3]=0;
	// 	usartBuffert[576+i*3+1]=i*8;
	// 	usartBuffert[576+i*3+2]=i*8;

	// 	// White
	// 	usartBuffert[672+i*3]=i*8;
	// 	usartBuffert[672+i*3+1]=i*8;
	// 	usartBuffert[672+i*3+2]=i*8;
	// }

	for(int n=0; n<24480; n++){
		arrayBuffert0[n]=0;
	}

	makeBigBuffer( );

 // Current buffer is 0, so data was loaded into 1.
	for(int i=0; i<24480; i++){
		arrayBuffert0[i] = arrayBuffert1[i];
	}

 // The first data to be sent out.
	for(int i=0; i<12; i++){
		spiTxBuf[i] = arrayBuffert0[i];
	}	
}
void makeBigBuffer( ){

	uint8_t oneFrame[96];		   
	volatile uint16_t ledPos	= 0;
	volatile uint8_t andVal	 = 0;
	volatile uint16_t writeBit  = 0;
	volatile uint32_t writeByte = 0;
	frameByte = 0;

	for(int bit=1; bit<9; bit++){

	 // Delete last iteration.
		for(int i = 0; i<96; i++){
			oneFrame[i]=0;
		}

		writeByte = 0;
		writeBit = 0;

	 // Add bits for every LED.
		for(int led=0; led<256; led++){

			ledPos = led*3;
			andVal = (1<<(bit-1));

		 // If red.
			if(usartBuffert[ledPos] & andVal){		  //  Max 765
				oneFrame[writeByte] += (128>>writeBit);
			}

			writeBit++;
			if(writeBit>=8){
				writeBit = 0;
				writeByte++;
			}

		 // If green.
			if(usartBuffert[ledPos+1] & andVal){
				oneFrame[writeByte] += (128>>writeBit);
			}

			writeBit++;
			if(writeBit>=8){
				writeBit = 0;
				writeByte++;
			}

		 // If blue.
			if(usartBuffert[ledPos+2] & andVal){
				oneFrame[writeByte] += (128>>writeBit);
			}

			writeBit++;
			if(writeBit>=8){
				writeBit = 0;
				writeByte++;
			}			
			}

	 // Write to buffer.
		if(currentPointer == 0){
			for(int i = (1<<(bit-1)); i<(1<<bit); i++){	 

				for(int j = 0; j<96; j++){
					arrayBuffert1[frameByte] = oneFrame[j];
					frameByte++;
				}
			}
		}
		else{

			for(int i = (1<<(bit-1)); i<(1<<bit); i++){	 

					for(int j = 0; j<96; j++){
					arrayBuffert0[frameByte] = oneFrame[j];
					frameByte++;
				}
			}
		}	
	}
}
