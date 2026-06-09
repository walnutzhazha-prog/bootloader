#ifndef __INT_BOOTLOADER_H
#define __INT_BOOTLOADER_H

#include "usart.h"
#include "stdlib.h"
#include "string.h"

#define BOOTLOADER_UART_REC_BUFF_LEN 1024

#define RESET_START 0x08004000
#define APP_START 0x08008000
#define STACK_ADDR 0x20000000
#define APP_END_ADDR 0x08080000



/**
 *@brief 跳转到A程序
 uint8_t 0:成功  1:失败
 *
 */
uint8_t Int_bootloader_jump_to_app(uint32_t app_start_addr);

#endif
