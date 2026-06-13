#include "Int_w24c02.h"


/**
 * @brief 读取一个字节
 * 
 * @param byte_addr 字节地址
 * @return uint8_t 读取到的数据
 */
uint8_t Int_w24c02_read_byte(uint8_t byte_addr)
{
    uint8_t data;
    //1.流程：启动信号 -> 发送写从设备地址 -> 发送字节地址 -> 启动信号 -> 发送读从设备地址  -> 读取数据 -> NACK 发送停止信号
    //(1)句柄编号 I2C1  (2)设备地址  芯片固定值 (3)字节地址 数据写入的位置  (4)地址长度 (5)数据存放地址  (6)数据长度 (7)超时时间 100ms
    HAL_I2C_Mem_Read(&hi2c1,W24C02_ADDR_R,byte_addr,I2C_MEMADD_SIZE_8BIT,&data,1,100);
    return data;
}
/**
 * @brief 写入一个字节
 * 
 * @param byte_addr 字节地址
 * @param data 要写入的数据
 */
void Int_w24c02_write_byte(uint8_t byte_addr, uint8_t data)
{
    //1.流程：启动信号  -> 发送写从设备地址 -> 发送字节地址 -> 发送数据 -> 发送停止信号
    //(1)句柄编号 I2C1  (2)设备地址  芯片固定值 (3)字节地址 数据写入的位置  (4)地址长度 8位 (5)数据存放地址  (6)数据长度 (7)超时时间 100ms
    HAL_I2C_Mem_Write(&hi2c1, W24C02_ADDR, byte_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

/**
 * @brief 读取多个字节
 * 
 * @param byte_addr 字节地址
 * @param data 读取到的数据
 * @param len 要读取的字节数
 */
void Int_w24c02_read_bytes(uint8_t byte_addr, uint8_t *data, uint16_t len)
{

    //1.流程：启动信号 -> 发送写从设备地址 -> 发送字节地址 -> 启动信号 -> 发送读从设备地址  -> 读取数据 -> ACK -> 读取数据 -> ...读取最后一个数据 NACK 发送停止信号
    //(1)句柄编号 I2C1  (2)设备地址  芯片固定值 (3)字节地址 数据写入的位置  (4)地址长度 (5)数据存放地址  (6)数据长度 (7)超时时间 100ms
    HAL_I2C_Mem_Read(&hi2c1, W24C02_ADDR_R, byte_addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);

}

/**
 * @brief 写入多个字节
 * EEPROM中 一次最多只能写入一页 （16字节）     0x00 -> 0x10  从0x05开始写 -> 0x10 也一样只能写到0x10
 * @param byte_addr 字节地址
 * @param data 要写入的数据
 * @param len 要写入的字节数
 */


/*
 void Int_w24c02_write_bytes(uint8_t byte_addr, uint8_t *data, uint16_t len)
{
    //1.流程：启动信号  -> 发送写从设备地址 -> 发送字节地址 -> 发送数据 -> 发送数据 -> ...发送停止信号
    //(1)句柄编号 I2C1  (2)设备地址  芯片固定值 (3)字节地址 数据写入的位置  (4)地址长度 8位 (5)数据存放地址  (6)数据长度 (7)超时时间 100ms
    //HAL_I2C_Mem_Write(&hi2c1, W24C02_ADDR, byte_addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);


    //实现多段写入的效果  一页写满之后继续写入下一页
    //(1)循环单字节写入 => 实现代码简单   效率低
    //(2)软件判断写入具体哪几页  1页写入1次

    //1.0 健壮性判断  地址值不能超过EEPROM的地址值
    if(byte_addr + len > 255)
    {
        printf("写入的地址值超过EEPROM的地址值\r\n");
        return;
    }


    //1.1判断当前一页的剩余空间
    uint8_t page_remain_len = W24C02_PAGE_SIZE - (byte_addr % W24C02_PAGE_SIZE);
    
    if(len <= page_remain_len) {
        // 可以一次写完
        HAL_I2C_Mem_Write(&hi2c1, W24C02_ADDR, byte_addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
    } 
    else 
    {
        //下次写入的起始地址
        uint8_t page_start_addr = byte_addr;
        //已经写入的页数
        uint8_t page_count = 0;
        // 不能一次写完
        while (len > page_remain_len)
        {
            //将当前页剩余的空间写满
            HAL_I2C_Mem_Write(&hi2c1, W24C02_ADDR, page_start_addr, I2C_MEMADD_SIZE_8BIT, data + page_count * W24C02_PAGE_SIZE, page_remain_len, 1000);

            //下一页的起始地址  一页的开头
            page_count++;
            page_start_addr += page_remain_len;
            //数据还剩下的长度
            len -= page_remain_len;
            //当前页剩余的空间
            page_remain_len = W24C02_PAGE_SIZE;
            //EEPROM每次写入以后需要等待5ms以上
            HAL_Delay(10);
        }
        //最后一页写入
        if(len != 0)
        {
            HAL_I2C_Mem_Write(&hi2c1, W24C02_ADDR, page_start_addr, I2C_MEMADD_SIZE_8BIT, data + page_count * W24C02_PAGE_SIZE, len, 1000);
        }
    }
}
*/

void Int_w24c02_write_bytes(uint8_t byte_addr, uint8_t *data, uint16_t len)
{
    // 1.0 健壮性判断：地址值不能超过 EEPROM 的最大容量 (256字节，索引 0~255)
    if (byte_addr + len > 256)
    {
        printf("Error: 写入的长度超出 EEPROM 最大边界！\r\n");
        return;
    }

    // AT24C02 的硬件单页大小为 8 字节
    const uint8_t PAGE_SIZE = 8; 

    // 1.1 计算当前地址在第一页里还剩下多少个字节可写
    uint8_t page_remain_len = PAGE_SIZE - (byte_addr % PAGE_SIZE);
    
    // 记录已经成功写入的总字节数，用于控制 data 指针的偏移
    uint16_t written_count = 0; 
    
    uint8_t current_addr = byte_addr;

    if (len <= page_remain_len) 
    {
        // 如果剩余空间足够，直接一次性写完
        HAL_I2C_Mem_Write(&hi2c1, W24C02_ADDR, current_addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
        HAL_Delay(5); // 必须等待 EEPROM 内部擦写完成
    } 
    else 
    {
        // 如果不够写，先写满当前页的剩余空间
        HAL_I2C_Mem_Write(&hi2c1, W24C02_ADDR, current_addr, I2C_MEMADD_SIZE_8BIT, data, page_remain_len, 1000);
        HAL_Delay(5); // 每次写入后必须延时 5ms 

        current_addr += page_remain_len;   // 更新下一个写入的物理地址
        written_count += page_remain_len;  // 更新已写字节数
        len -= page_remain_len;            // 扣除已写长度

        // 循环写入后续的完整页
        while (len >= PAGE_SIZE)
        {
            // 精准控制数据指针：data + written_count
            HAL_I2C_Mem_Write(&hi2c1, W24C02_ADDR, current_addr, I2C_MEMADD_SIZE_8BIT, data + written_count, PAGE_SIZE, 1000);
            HAL_Delay(5); 

            current_addr += PAGE_SIZE;
            written_count += PAGE_SIZE;
            len -= PAGE_SIZE;
        }
        
        // 最后一页：写入零头数据（不足一整页的部分）
        if (len > 0)
        {
            HAL_I2C_Mem_Write(&hi2c1, W24C02_ADDR, current_addr, I2C_MEMADD_SIZE_8BIT, data + written_count, len, 1000);
            HAL_Delay(5); 
        }
    }
}

