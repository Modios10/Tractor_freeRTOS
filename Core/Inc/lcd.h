#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

/* Dirección típica del módulo I2C */
#define LCD_ADDR (0x27 << 1)

/* Bits PCF8574 */
#define LCD_RS 0x01
#define LCD_EN 0x04
#define LCD_BL 0x08

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);

void LCD_SendString(char *str);
void LCD_Print(char *line1, char *line2);

#endif