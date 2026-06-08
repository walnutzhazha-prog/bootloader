#ifndef __INT_BOOTLOADER_H
#define __INT_BOOTLOADER_H

#include "usart.h"
#include "stdlib.h"
#include "string.h"

#define BOOTLOADER_UART_REC_BUFF_LEN    1024 

//程序写入的起始位置 => A程序起始位置在 0x08008000  
#define APP_START_ADDR    0x08008000
#define APP_SIZE          0x7C000
#define STACK_ADDR        0x20000000
#define APP_END_ADDR      0x08080000


/**
 *@brief 串口接收 => 准备接收A程序
 *
 */
void Int_bootloader_receive_app(void);

/**
 *@brief 跳转到A程序
 uint8_t 0:成功  1:失败
 *
 */
uint8_t Int_bootloader_jump_to_app(void);

/**
 * @brief 外部可调用 提前擦出flash
 * 
 */
void Int_bootloader_erase_flash(uint32_t page_addr,uint16_t pages);

#endif
