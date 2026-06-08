#ifndef __APP__BOOTLOADER_H
#define __APP__BOOTLOADER_H

#include "Int_w24c02.h"
#include "Int_bootloader.h"


//添加校验的密钥
#define CHECK_KEY_ADDR 0x11
#define CHECK_KEY      0x5A6B
//存储更新状态的位置
#define CHECK_UPDATE_ADDR 0x10
//更新状态的值
#define BOOT_UPDATE 0x01
#define BOOT_NO_UPDATE 0x02
#define BOOT_RESET 0x03




/**
 * @brief 判断当前是否需要更新
 * 
 */
void App_bootloader_check_update(void);


/**
 * @brief 判断当前是否需要恢复出厂设置
 * 
 */
void App_bootloader_check_default(void);


/**
 * @brief 执行更新操作
 * 
 */
void App_bootloader_update();


/**
 * @brief 执行跳转操作
 * 
 */
void App_bootloader_jump_app();

#endif

