#include "lcd.h"
#include "user_i2c.h"

/* ------------------------------------------------ */
/* Bajo nivel I2C                                  */
/* ------------------------------------------------ */

static void LCD_Write(uint8_t data)
{
    USER_I2C1_Start();
    USER_I2C1_Write_Address(LCD_ADDR);
    USER_I2C1_Write(data | LCD_BL);
    USER_I2C1_Stop();
}

static void LCD_Pulse(uint8_t data)
{
    LCD_Write(data | LCD_EN);
    for(volatile int i=0;i<200;i++);

    LCD_Write(data & ~LCD_EN);
    for(volatile int i=0;i<200;i++);
}

static void LCD_Send4(uint8_t data)
{
    LCD_Write(data);
    LCD_Pulse(data);
}

static void LCD_Send(uint8_t value, uint8_t mode)
{
    uint8_t high = (value & 0xF0) | mode;
    uint8_t low  = ((value << 4) & 0xF0) | mode;

    LCD_Send4(high);
    LCD_Send4(low);

    /* HD44780 needs extra time after clear/home commands. */
    if(mode == 0x00)
    {
        if((value == 0x01) || (value == 0x02))
        {
            for(volatile int i = 0; i < 12000; i++);
        }
        else
        {
            for(volatile int i = 0; i < 1200; i++);
        }
    }
    else
    {
        for(volatile int i = 0; i < 800; i++);
    }
}

/* ------------------------------------------------ */
/* API pública                                     */
/* ------------------------------------------------ */

void LCD_Init(void)
{
    for(volatile int i=0;i<150000;i++);

    LCD_Send4(0x30);
    for(volatile int i=0;i<50000;i++);

    LCD_Send4(0x30);
    for(volatile int i=0;i<50000;i++);

    LCD_Send4(0x30);
    for(volatile int i=0;i<50000;i++);

    LCD_Send4(0x20);
    for(volatile int i=0;i<50000;i++);

    LCD_Send(0x28, 0x00); // 4 bits, 2 líneas
    LCD_Send(0x0C, 0x00); // display ON
    LCD_Send(0x06, 0x00); // entry mode
    LCD_Send(0x01, 0x00); // clear

    for(volatile int i=0;i<80000;i++);
}

void LCD_Clear(void)
{
    LCD_Send(0x01, 0x00);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? 0x80 + col : 0xC0 + col;
    LCD_Send(addr, 0x00);
}

void LCD_SendString(char *str)
{
    while(*str)
    {
        LCD_Send(*str++, LCD_RS);
    }
}

void LCD_Print(char *line1, char *line2)
{
    LCD_SetCursor(0,0);
    LCD_SendString(line1);

    LCD_SetCursor(1,0);
    LCD_SendString(line2);
}