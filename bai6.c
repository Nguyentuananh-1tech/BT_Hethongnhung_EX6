#include "stm32f10x.h"
#include <stdio.h>

#define DS3231_ADDR 0x68

void I2C1_Init(void);
void USART2_Init(void);
void UART2_SendChar(char c);
void UART2_SendString(char *str);

uint8_t I2C1_ReadRegister(uint8_t reg);
uint8_t bcd_to_dec(uint8_t val);

int main(void)
{
    char buffer[50];
    uint8_t sec, min, hour, day, month, year;

    I2C1_Init();
    USART2_Init();

    UART2_SendString("Doc thoi gian tu DS3231...\r\n");

    while (1)
    {
        sec   = bcd_to_dec(I2C1_ReadRegister(0x00));
        min   = bcd_to_dec(I2C1_ReadRegister(0x01));
        hour  = bcd_to_dec(I2C1_ReadRegister(0x02));
        day   = bcd_to_dec(I2C1_ReadRegister(0x04));
        month = bcd_to_dec(I2C1_ReadRegister(0x05));
        year  = bcd_to_dec(I2C1_ReadRegister(0x06));

        sprintf(buffer, "Time: %02d:%02d:%02d  Date: %02d/%02d/20%02d\r\n",
                hour, min, sec, day, month, year);
        UART2_SendString(buffer);

        for (volatile int i = 0; i < 800000; i++); 
    }
}


void I2C1_Init(void)
{
    GPIO_InitTypeDef gpio;
    I2C_InitTypeDef i2c;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;  
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    i2c.I2C_ClockSpeed = 100000;
    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1 = 0;
    i2c.I2C_Ack = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &i2c);

    I2C_Cmd(I2C1, ENABLE);
}

void USART2_Init(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    usart.USART_BaudRate = 9600;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &usart);
    USART_Cmd(USART2, ENABLE);
}

void UART2_SendChar(char c)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    USART_SendData(USART2, c);
}

void UART2_SendString(char *str)
{
    while (*str)
    {
        UART2_SendChar(*str++);
    }
}

uint8_t I2C1_ReadRegister(uint8_t reg)
{
    uint8_t value;
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, DS3231_ADDR << 1, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, reg);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, DS3231_ADDR << 1, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    value = I2C_ReceiveData(I2C1);

    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_GenerateSTOP(I2C1, ENABLE);

    return value;
}

uint8_t bcd_to_dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0F);
}
