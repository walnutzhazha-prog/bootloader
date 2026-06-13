#ifndef __INT_W24C02_H__
#define __INT_W24C02_H__

#include "i2c.h"
#include "usart.h"

#define W24C02_ADDR 0xA0
#define W24C02_ADDR_R (W24C02_ADDR | 0x01)

#define W24C02_ADDR_SIZE 8
//#define W24C02_PAGE_SIZE 8

/**
 * @brief 读取一个字节
 * 
 * @param byte_addr 字节地址
 * @return uint8_t 读取到的数据
 */
uint8_t Int_w24c02_read_byte(uint8_t byte_addr);

/**
 * @brief 写入一个字节
 * 
 * @param byte_addr 字节地址
 * @param data 要写入的数据
 */
void Int_w24c02_write_byte(uint8_t byte_addr, uint8_t data);

/**
 * @brief 读取多个字节
 * 
 * @param byte_addr 字节地址
 * @param data 读取到的数据
 * @param len 要读取的字节数
 */
void Int_w24c02_read_bytes(uint8_t byte_addr, uint8_t *data, uint16_t len);

/**
 * @brief 写入多个字节
 * 
 * @param byte_addr 字节地址
 * @param data 要写入的数据
 * @param len 要写入的字节数
 */
void Int_w24c02_write_bytes(uint8_t byte_addr, uint8_t *data, uint16_t len);

#endif
