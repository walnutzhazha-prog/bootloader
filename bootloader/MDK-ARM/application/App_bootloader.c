#include "App_bootloader.h"

uint8_t app_boot_update_status = BOOT_NO_UPDATE;

/**
 * @brief 判断当前是否需要更新
 * 
 */
void App_bootloader_check_update(void)
{
    //读取3个字节的数据
    uint8_t data[3];
    Int_w24c02_read_bytes(CHECK_KEY_ADDR, data, 3);
    // 1.校验密钥是否正确      高8位在前
    uint16_t key = data[1] << 8 | data[2];
    if(key != CHECK_KEY)
    {
        // 2.密钥不正确，不进行更新  重置密钥
        data[0] = BOOT_NO_UPDATE;
        data[1] = (uint8_t)CHECK_KEY >> 8;
        data[2] = (uint8_t)CHECK_KEY;
        Int_w24c02_write_bytes(CHECK_UPDATE_ADDR, data, 3);
        HAL_Delay(10);
    }
    else
    {
        // 3.密钥正确，读取状态 判断当前是否需要更新
        app_boot_update_status = data[0];
    }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == KEY1_Pin)
    {
        // 重置按钮按下，执行恢复出厂设置
        app_boot_update_status = BOOT_RESET;
    }
}

/**
 * @brief 判断当前是否需要恢复出厂设置
 * 
 */
void App_bootloader_check_default(void)
{
    HAL_Delay(3000);
}


/**
 * @brief 执行更新操作
 * 
 */
void App_bootloader_update()
{
    if(app_boot_update_status == BOOT_UPDATE)
    {
        //将W25Q128中的程序写入到flash中
        printf("app update\r\n");
    }
    else if(app_boot_update_status == BOOT_NO_UPDATE)
    {
        printf("no update\r\n");
    }
    else if(app_boot_update_status == BOOT_RESET)
    {
        printf("reset\r\n");
    }
}


/**
 * @brief 执行跳转操作
 * 
 */
void App_bootloader_jump_app()
{
    //不管更新与否，最后都需要执行的跳转操作 到A程序中
    Int_bootloader_jump_to_app();
}
