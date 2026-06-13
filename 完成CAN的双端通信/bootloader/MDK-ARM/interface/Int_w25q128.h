#ifndef __INT_W25Q128_H
#define __INT_W25Q128_H

#include "spi.h"

#define W25Q128_READ_ID 0x90
#define W25Q128_READ_STATUS_REG 0x05
#define W25Q128_READ_DATA 0x03
#define W25Q128_WRITE_DATA 0x02
#define W25Q128_ERASE_SECTOR 0x20
#define W25Q128_WRITE_ENABLE 0x06

/**
 * SPI设备在使用的时候片选引脚才需要拉低，平时都需要拉高
 * @brief 启动W25Q128的SPI设备
 * @note 该函数会将片选引脚拉低，表示SPI设备被选中，可以进行通信。使用完毕后需要调用Int_w25q128_stop()函数将片选引* 脚拉高，表示SPI设备不再被选中。
 */
void Int_w25q128_start(void);

/**
 * @brief 拉高片选引脚，停止与W25Q128的通信
 *
 */
void Int_w25q128_stop(void);

/**
 * @brief 写入一个字节
 * @param data
 */
void Int_w25q128_write_byte(uint8_t data);

/**
 * @brief 读取一个字节
 *
 */
uint8_t Int_w25q128_read_byte(void);

/**
 * @brief 读取W25Q128的ID
 * @param mf_id 生产厂商ID
 * @param device_id 设备ID
 */
void Int_w25q128_read_id(uint8_t *mf_id, uint16_t *device_id);

/**
 * @brief 读取数据
 * @param data
 * addr 是W25Q128的地址  0-0xFFFFFF   一次擦出4096字节  一次写入256字节
 */
void Int_w25q128_read_data(uint8_t block, uint8_t sector, uint8_t page, uint8_t addr, uint8_t *data, uint16_t len);

/**
 * @brief 读取数据 * addr 是W25Q128的地址  0-0xFFFFFF   一次擦
 *
 * @param addr 读取数据  使用32位地址
 * @param data 数据缓冲区
 * @param len 读取数据的长度
 */
void Int_w25q128_read_data_with_32addr(uint32_t addr, uint8_t *data, uint16_t len);

/**
 * @brief 写入数据
 *
 */
void Int_w25q128_write_data(uint8_t block, uint8_t sector, uint8_t page, uint8_t addr, uint8_t *data, uint16_t len);

/**
 * @brief 擦除一扇区
 *
 */
void Int_w25q128_erase_sector(uint8_t block, uint8_t sector);

#endif
