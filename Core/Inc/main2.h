#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#include "main.h"


//System clock configuration function to 64MHz
void USER_SystemClock_Config( void ){

	// FLASH latency = 2 wait states
	FLASH->ACR &= ~( 0x7UL << 0U );
	FLASH->ACR |=  ( 0x2UL << 0U );

	// APB1 prescaler = /2
	// APB2 prescaler = /1
	// ADC prescaler = /6
	RCC->CFGR &= ~(( 0x7UL << 8U  ) |
				   ( 0x7UL << 11U ) |
				   ( 0x3UL << 14U ));

	RCC->CFGR |=  (( 0x4UL << 8U  ) |   // APB1 /2
				   ( 0x2UL << 14U ));  // ADC /6

	// PLL source = HSI/2
	RCC->CFGR &= ~( 0x1UL << 16U );

	// PLL multiplier = x16
	RCC->CFGR &= ~( 0xFUL << 18U );
	RCC->CFGR |=  ( 0xFUL << 18U );

	// Enable PLL
	RCC->CR |= ( 0x1UL << 24U );

	// Wait until PLL ready
	while( !(RCC->CR & ( 0x1UL << 25U )));

	// Select PLL as system clock
	RCC->CFGR &= ~( 0x3UL << 0U );
	RCC->CFGR |=  ( 0x2UL << 0U );

	// Wait until PLL selected as system clock
	while(( RCC->CFGR & ( 0x3UL << 2U )) != ( 0x2UL << 2U ));
}



//GPIOA configuration function
void GPIOA_init( void ){

	// GPIOA clock enable
	RCC->APB2ENR |= ( 0x1UL << 2U );
}



//USART1 configuration function
void USART1_Init( void ){

    // USART1 clock enable
    RCC->APB2ENR |= ( 0x1UL << 14U );

    // PA9 (TX) configured as alternate function push-pull
    GPIOA->CRH &= ~( 0xFUL << 4U );
    GPIOA->CRH |=  ( 0xBUL << 4U );

    // PA10 (RX) configured as input floating (NUEVO - Vital para escuchar al ESP)
    GPIOA->CRH &= ~( 0xFUL << 8U );
    GPIOA->CRH |=  ( 0x4UL << 8U );

    // USART enable
    USART1->CR1 |= ( 0x1UL << 13U );

    // Transmitter enable (TE)
    USART1->CR1 |= ( 0x1UL << 3U );

    // Receiver enable (RE) (NUEVO - Enciende el receptor interno)
    USART1->CR1 |= ( 0x1UL << 2U );

    // 8-bit word length
    USART1->CR1 &= ~( 0x1UL << 12U );

    // 1 stop bit
    USART1->CR2 &= ~( 0x3UL << 12U );

    //=========================================================
    // Baud Rate Configuration
    // 64MHz clock / 19200 baud
    //
    // USARTDIV = 208.333
    // Mantissa = 208
    // Fraction = 5
    //=========================================================
// --> ADD THIS EXACT LINE: Set priority to 5 (Safe for FreeRTOS)
    NVIC_SetPriority(USART1_IRQn, 5);

    USART1->BRR = 0;
    USART1->BRR |= ( 208UL << 4U );
    USART1->BRR |= ( 5UL << 0U );

	// 1. Activar la interrupción de RXNE (Registro de recepción no vacío) en el USART
    USART1->CR1 |= ( 0x1UL << 5U );

    // 2. Activar la interrupción de USART1 en el NVIC (Cerebro del ARM Cortex-M3)
    // El USART1 es la interrupción #37. Eso cae en el registro ISER[1], bit 5.
    *((volatile uint32_t *)0xE000E104UL) |= ( 0x1UL << 5U );
	NVIC_EnableIRQ(USART1_IRQn);
}


//USART2 configuration function
void USART2_Init( void ){

	// USART2 clock enable
	RCC->APB1ENR |= ( 0x1UL << 17U );

	// GPIOA clock enable
	RCC->APB2ENR |= ( 0x1UL << 2U );

	// PA2 configured as alternate function push-pull
	GPIOA->CRL &= ~( 0xFUL << 8U );
	GPIOA->CRL |=  ( 0xBUL << 8U );

	// PA3 configured as input floating
	GPIOA->CRL &= ~( 0xFUL << 12U );
	GPIOA->CRL |=  ( 0x4UL << 12U );

	// USART enable
	USART2->CR1 |= ( 0x1UL << 13U );

	// Transmitter enable
	USART2->CR1 |= ( 0x1UL << 3U );

	// 8-bit word length
	USART2->CR1 &= ~( 0x1UL << 12U );

	// 1 stop bit
	USART2->CR2 &= ~( 0x3UL << 12U );

	//=========================================================
	// Baud Rate Configuration
	// 32MHz APB1 clock / 115200 baud
	// USARTDIV = 17.361
	// Mantissa = 17
	// Fraction = 6
	//=========================================================

	USART2->BRR = 0;
	USART2->BRR |= ( 17UL << 4U );
	USART2->BRR |= ( 6UL << 0U );
}



void USART1_transmit( uint32_t data ){

	// Wait until transmit register empty
	while (!(USART1->SR & ( 0x1UL << 7U )));

	// Send data
	USART1->DR = data;
}


void USART2_transmit( uint32_t data ){

	// Wait until transmit register empty
	while (!(USART2->SR & ( 0x1UL << 7U )));

	// Send data
	USART2->DR = data;
}



// Sends an entire string of characters over USART
void USART1_print(const char* str) {

    while (*str != '\0') {

        USART1_transmit(*str);
        str++;
    }
}


void USART2_print(const char* str) {

	while (*str != '\0') {

		USART2_transmit(*str);
		str++;
	}
}



// Converts a raw number into ASCII characters and sends it
void USART1_printNumber(uint32_t val) {

    char buffer[10];
    int i = 0;

    // Handle 0 explicitly
    if (val == 0) {

        USART1_transmit('0');
        return;
    }

    // Convert integer to ASCII
    while (val > 0) {

        buffer[i] = (val % 10) + '0';
        val /= 10;
        i++;
    }

    // Send in correct order
    while (i > 0) {

        i--;
        USART1_transmit(buffer[i]);
    }
}


void USART2_printNumber(uint32_t val) {

	char buffer[10];
	int i = 0;

	// Handle 0 explicitly
	if (val == 0) {

		USART2_transmit('0');
		return;
	}

	// Convert integer to ASCII
	while (val > 0) {

		buffer[i] = (val % 10) + '0';
		val /= 10;
		i++;
	}

	// Send in correct order
	while (i > 0) {

		i--;
		USART2_transmit(buffer[i]);
	}
}



//ADC1 configuration function
void ADC1_Init( void ){

	// ADC1 clock enable
	RCC->APB2ENR |= ( 0x1UL << 9U );

	// PA7 configured as analog input
	GPIOA->CRL &= ~( 0xFUL << 28U );


	//---------------------------------------------------------
	// ADC regular sequence = 1 conversion
	//---------------------------------------------------------
	ADC1->SQR1 &= ~( 0xFUL << 20U );


	//---------------------------------------------------------
	// Channel 7 selected
	//---------------------------------------------------------
	ADC1->SQR3 &= ~( 0x1FUL << 0U );
	ADC1->SQR3 |=  ( 0x7UL << 0U );


	//---------------------------------------------------------
	// Sample time channel 7 = 239.5 cycles
	//---------------------------------------------------------
	ADC1->SMPR2 &= ~( 0x7UL << 21U );
	ADC1->SMPR2 |=  ( 0x7UL << 21U );


	//---------------------------------------------------------
	// Right alignment
	//---------------------------------------------------------
	ADC1->CR2 &= ~( 0x1UL << 11U );


	//---------------------------------------------------------
	// ADC ON
	//---------------------------------------------------------
	ADC1->CR2 |= ( 0x1UL << 0U );


	// Small stabilization delay
	for(volatile int i = 0; i < 10000; i++);


	//---------------------------------------------------------
	// Reset calibration
	//---------------------------------------------------------
	ADC1->CR2 |= ( 0x1UL << 3U );

	while(ADC1->CR2 & ( 0x1UL << 3U ));


	//---------------------------------------------------------
	// Start calibration
	//---------------------------------------------------------
	ADC1->CR2 |= ( 0x1UL << 2U );

	while(ADC1->CR2 & ( 0x1UL << 2U ));
}



uint32_t ADC1_read( void ){

	// Start conversion
	ADC1->CR2 |= ( 0x1UL << 22U );

	// Wait until conversion complete
	while (!(ADC1->SR & ( 0x1UL << 1U )));

	// Return ADC result
	uint32_t result = ADC1->DR;

	return result;
}



//TIM2 configuration function
void TIM2_Init( void ){

	// TIM2 clock enable
	RCC->APB1ENR |= ( 0x1UL << 0U );
}



//Simple delay
void USER_Delay_1sec( void ){

	for(volatile uint32_t i = 0; i < 7000000UL; i++);
}


#endif