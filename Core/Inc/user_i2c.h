#ifndef USER_I2C_H_
#define USER_I2C_H_

#include <stdint.h>

/*
 * 1 = I2C1 remapped to PB8/PB9
 * 0 = I2C1 on default PB6/PB7
 */
#define USER_I2C1_REMAP_PINS 0

void USER_I2C1_Init(void);

void USER_I2C1_Start(void);

void USER_I2C1_Stop(void);

void USER_I2C1_Write(uint8_t data);

void USER_I2C1_Write_Address(uint8_t address);

#endif