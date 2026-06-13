#include "Int_w25q128.h"

/**
 * SPI设备在使用的时候片选引脚才需要拉低，平时都需要拉高
 * @brief 启动W25Q128的SPI设备
 *
 */
void Int_w25q128_start(void)
{
    HAL_GPIO_WritePin(W25Q128_CS_GPIO_Port, W25Q128_CS_Pin, GPIO_PIN_RESET);
}

/**
 * @brief 拉高片选引脚，停止与W25Q128的通信
 *
 */
void Int_w25q128_stop(void)
{
    HAL_GPIO_WritePin(W25Q128_CS_GPIO_Port, W25Q128_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief 写入一个字节
 * @param data
 */
void Int_w25q128_write_byte(uint8_t data)
{
    HAL_SPI_Transmit(&hspi2, &data, 1, 100);
}

/**
 * @brief 读取一个字节
 *
 */
uint8_t Int_w25q128_read_byte(void)
{
    uint8_t data;
    HAL_SPI_Receive(&hspi2, &data, 1, 100);
    return data;
}

/**
 * @brief 读取W25Q128的ID
 * @param mf_id 生产厂商ID
 * @param device_id 设备ID
 */
void Int_w25q128_read_id(uint8_t *mf_id, uint16_t *device_id)
{
    // 1.拉低片选引脚，启动SPI设备
    Int_w25q128_start();
    // 2.发送读取ID的命令
    Int_w25q128_write_byte(W25Q128_READ_ID);

    Int_w25q128_write_byte(0x00);
    Int_w25q128_write_byte(0x00);
    Int_w25q128_write_byte(0x00);

    // 3.读取生产厂商ID和设备ID
    *mf_id = Int_w25q128_read_byte();
    *device_id = (uint16_t)Int_w25q128_read_byte();

    // 4.拉高片选引脚，停止SPI设备
    Int_w25q128_stop();
}

// 静态方法  等待芯片忙状态
static void Int_w25q128_wait_busy(void)
{
    // 1.拉低片选引脚，启动SPI设备
    Int_w25q128_start();

    // 2.读取寄存器状态
    while (1)
    {
        Int_w25q128_write_byte(W25Q128_READ_STATUS_REG); // 读取状态寄存器的命令
        uint8_t status = Int_w25q128_read_byte();
        // 找到busy是最低位的值  为0表示不忙
        if ((status & 0x01) == 0)
        {
            break; // 芯片不忙了，退出循环
        }
    }

    // 3.拉高片选引脚，停止SPI设备
    Int_w25q128_stop();
}

/**
 * @brief 读取数据
 * @param data
 * addr 是W25Q128的地址  0-0xFFFFFF   一次擦出4096字节  一次写入256字节
 */
void Int_w25q128_read_data(uint8_t block, uint8_t sector, uint8_t page, uint8_t addr, uint8_t *data, uint16_t len)
{
    // 1.等待忙状态
    Int_w25q128_wait_busy(); // 等待芯片忙状态

    // 2.拉低片选引脚，启动SPI设备
    Int_w25q128_start();

    // 3.发送读取数据的命令和地址
    Int_w25q128_write_byte(W25Q128_READ_DATA); // 读取数据的命令
    uint32_t addr_24 = ((uint32_t)block << 16) | ((uint32_t)sector << 12) | ((uint32_t)page << 8) | addr;
    Int_w25q128_write_byte((addr_24 >> 16)); // 地址的高8位
    Int_w25q128_write_byte((addr_24 >> 8));  // 地址的中8位
    Int_w25q128_write_byte(addr_24);         // 地址的低8位
    for (int i = 0; i < len; i++)
    {
        data[i] = Int_w25q128_read_byte();
    }
    // 4.拉高片选引脚，停止SPI设备
    Int_w25q128_stop();
}


/**
 * @brief 读取数据 * addr 是W25Q128的地址  0-0xFFFFFF   一次擦
 * 
 * @param addr 读取数据  32位地址
 * @param data 数据缓冲区
 * @param len 读取数据的长度
 */
void Int_w25q128_read_data_with_32addr(uint32_t addr, uint8_t *data, uint16_t len)
{
    // 1.等待忙状态
    Int_w25q128_wait_busy(); // 等待芯片忙状态

    // 2.拉低片选引脚，启动SPI设备
    Int_w25q128_start();

    // 3.发送读取数据的命令和地址
    Int_w25q128_write_byte(W25Q128_READ_DATA); // 读取数据的命令

    Int_w25q128_write_byte((addr >> 16) & 0xff); // 地址的高8位  
    Int_w25q128_write_byte((addr >> 8) & 0xff);     
    Int_w25q128_write_byte(addr & 0xff);            
    for (int i = 0; i < len; i++)
    {
        data[i] = Int_w25q128_read_byte();
    }
    // 4.拉高片选引脚，停止SPI设备
    Int_w25q128_stop();
}


static void Int_w25q128_write_enable(void)
{
    // 1.等待忙状态
    Int_w25q128_wait_busy();
    // 2.拉低片选引脚，启动SPI设备
    Int_w25q128_start();
    // 3.发送写使能的命令
    Int_w25q128_write_byte(W25Q128_WRITE_ENABLE);
    // 4.拉高片选引脚，停止SPI设备
    Int_w25q128_stop();
}

/**
 * @brief 写入数据
 * 假设地址不超过1页范围
 */
void Int_w25q128_write_data(uint8_t block, uint8_t sector, uint8_t page, uint8_t addr, uint8_t *data, uint16_t len)
{
    
    // 1.写使能
    Int_w25q128_write_enable();
    // 2.拉低片选引脚，启动SPI设备
    Int_w25q128_start();
    uint32_t addr_24 = ((uint32_t)block << 16) | ((uint32_t)sector << 12) | ((uint32_t)page << 8) | addr;
    Int_w25q128_write_byte(W25Q128_WRITE_DATA);
    Int_w25q128_write_byte((addr_24 >> 16)); // 地址的高8位
    Int_w25q128_write_byte((addr_24 >> 8));  // 地址的中8位
    Int_w25q128_write_byte(addr_24);         // 地址的低8位
    // 3.发送写入数据的命令和地址
    for (int i = 0; i < len; i++)
    {
        Int_w25q128_write_byte(data[i]);
    }

    // 4.拉高片选引脚，停止SPI设备
    Int_w25q128_stop();
    // 5.等待忙状态
    Int_w25q128_wait_busy();
}

/**
 * @brief 使用32位地址直接写入 1次最多写1页
 * 
 */
void Int_w25q128_write_data_with_32addr(uint32_t addr, uint8_t *data, uint16_t len)
{

    // 1.写使能
    Int_w25q128_write_enable();
    // 2.拉低片选引脚，启动SPI设备
    Int_w25q128_start();
    Int_w25q128_write_byte(W25Q128_WRITE_DATA);
    Int_w25q128_write_byte((addr >> 16) & 0xff);
    Int_w25q128_write_byte((addr >> 8) & 0xff);
    Int_w25q128_write_byte(addr & 0xff);
    // 3.发送写入数据的命令和地址
    for (int i = 0; i < len; i++)
    {
        Int_w25q128_write_byte(data[i]);
    }

    // 4.拉高片选引脚，停止SPI设备
    Int_w25q128_stop();
    // 5.等待忙状态
    Int_w25q128_wait_busy();
}

/**
 * @brief 擦除一扇区
 *
 */
void Int_w25q128_erase_sector(uint8_t block, uint8_t sector)
{
    // 1.写使能
    Int_w25q128_write_enable();
    // 2.拉低片选引脚，启动SPI设备
    Int_w25q128_start();
    uint32_t addr = (uint32_t)block * 65536 + (uint32_t)sector * 4096;
    // 3.发送扇区擦除的命令和地址
    Int_w25q128_write_byte(W25Q128_ERASE_SECTOR);
    // 4.发送地址
    Int_w25q128_write_byte((addr >> 16) & 0xff);
    Int_w25q128_write_byte((addr >> 8) & 0xff);
    Int_w25q128_write_byte(addr & 0xff);
    // 5.拉高片选引脚，停止SPI设备
    Int_w25q128_stop();
}
