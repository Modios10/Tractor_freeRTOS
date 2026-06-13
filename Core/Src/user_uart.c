#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "user_uart.h"

static void USER_UART2_Send_8bit( uint8_t Data );

// The following makes printf() write to USART2:
int _write(int file, char *ptr, int len)
{
  int DataIdx;
  for( DataIdx = 0; DataIdx < len; DataIdx++ ){
    while(!( USART2->SR & ( 0x1UL <<  7U ) ));
    USART2->DR = *ptr++;
  }
  return len;
}

void USER_UART2_Init( void ){
  RCC->APB2ENR	|=	 ( 0x1UL <<  2U );//	IO port A clock enable
	
  //pin PA2 (USART2_TX) as alternate function output push-pull, max speed 10MHz
	GPIOA->CRL		&=	~( 0x1UL << 10U ) &	~( 0x2UL <<  8U );
	GPIOA->CRL		|=	 ( 0x2UL << 10U ) |	 ( 0x1UL <<  8U );

  // Configure the UART module
	RCC->APB1ENR	|=	 ( 0x1UL << 17U );//	USART 2 clock enable	
	USART2->CR1		|=	 ( 0x1UL << 13U );//	Step 1 Usart enabled
	USART2->CR1		&=	~( 0x1UL << 12U );//	Step 2 8 Data bits
	USART2->CR2		&=	~( 0x3UL << 12U );//	Step 3 1 Stop bit
	USART2->BRR	 	 =	   0x116;//			      Step 5 Desired baud rate (115200 baud)
	USART2->CR1		|= 	 ( 0x1UL <<  3U );//	Step 6 Transmitter enabled
}

static void USER_UART2_Send_8bit( uint8_t Data ){
	while(!( USART2->SR & ( 0x1UL <<  7U ) ));//	wait until next data can be written
	USART2->DR = Data;//				                  Step 7 Data to send
}

void USER_UART2_Transmit( uint8_t *pData, uint16_t size ){
	for( int i = 0; i < size; i++ ){
		USER_UART2_Send_8bit(*pData++ );
	}
}