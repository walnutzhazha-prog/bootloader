#include "App_bootloader.h"

uint8_t app_boot_update_status = BOOT_NO_UPDATE;

/**
 * @brief 判断当前是否需要更新
 *
 */
void App_bootloader_check_update(void)
{
    printf("bootloader start\r\n");
    printf("bootloader check update\r\n");
    // 读取3个字节的数据
    uint8_t data[3];
    Int_w24c02_read_bytes(CHECK_KEY_ADDR, data, 3);
    // 1.校验密钥是否正确      高8位在前
    uint16_t key = data[1] << 8 | data[2];
    if (key != CHECK_KEY)
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
    if (GPIO_Pin == KEY1_Pin)
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

uint8_t meta_app_buff[9] = {0};
// 程序在w25q128中保存的位置
uint32_t app_start_addr = 0;
// 需要写入到flash的程序大小
uint32_t app_size = 0;

//一次能够写入一页flash的缓冲区
uint8_t flash_data_buff[2049] = {0};

static void App_bootloader_check_meta_data(void)
{
    //  前4个字节是程序的起始地址   后4个字节是程序的大小   低位在前
    Int_w25q128_read_data(META_APP_ADDR_BLOCK, META_APP_ADDR_SECTOR, META_APP_ADDR_PAGE, META_APP_ADDR_ADDR, meta_app_buff, 8);
    app_start_addr = meta_app_buff[0] | meta_app_buff[1] << 8 | meta_app_buff[2] << 16 | meta_app_buff[3] << 24;
    app_size = meta_app_buff[4] | meta_app_buff[5] << 8 | meta_app_buff[6] << 16 | meta_app_buff[7] << 24;

    // 假设程序存储的地址不能在第一扇中 0x00 1 000
    if (app_start_addr < APP_START_ADDR_MIN)
    {
        printf("app start addr error! Read: 0x%08X, Expected > 0x%08X\r\n", app_start_addr, APP_START_ADDR_MIN);
        return;
    }
    if (app_size < APP_SIZE_MIN || app_size > APP_SIZE_MAX)
    {
        printf("app size error! Read: 0x%08X, Expected between 0x%08X and 0x%08X\r\n", app_size, APP_SIZE_MIN, APP_SIZE_MAX);
        return;
    }

    // 读取程序     判断头两个32位数据
    uint8_t app_data_tmp[2];
    Int_w25q128_read_data_with_32addr(app_start_addr, meta_app_buff, 8);
    uint32_t app_stack_ptr = meta_app_buff[0] | meta_app_buff[1] << 8 | meta_app_buff[2] << 16 | meta_app_buff[3] << 24;
    uint32_t app_reset_handle = meta_app_buff[4] | meta_app_buff[5] << 8 | meta_app_buff[6] << 16 | meta_app_buff[7] << 24;

    // 1.1校验栈顶地址
    if ((app_stack_ptr & 0xFFFF0000) != STACK_ADDR)
    {
        printf("stack addr error! Read MSP: 0x%08X, Expected High 16: 0x%04X\r\n", app_stack_ptr, STACK_ADDR);
        return;
    }

    // 1.2校验复位中断地址
    if ((app_reset_handle < APP_START) || (app_reset_handle > APP_END_ADDR))
    {
        printf("reset handle error\n");
        return;
    }
}

/**
 * @brief 直接擦除足够多的页数 
 *
 */
static void App_flash_erase(uint8_t pages)
{
    //  直接擦除足够的页大小
    FLASH_EraseInitTypeDef erase_init;
    // 擦除单独页
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    // 擦除第1个bank的页
    erase_init.Banks = FLASH_BANK_1;
    // 擦除页的起始地址
    erase_init.PageAddress = APP_START;
    // 擦除几页
    erase_init.NbPages = pages;
    uint32_t page_error = 0;
    // flash擦除比较耗费性能  => 需要优化  => 1.增加一个页擦除标志位  2.每次接收数据时先判断当前页是否已经擦除过了  如果没有擦除过了才擦除
    HAL_FLASHEx_Erase(&erase_init, &page_error);
}

static void app_bootloader_write_app_flash(void)
{
    // 1.读取元数据信息 => 描述后续的程序

    // 2.校验程序
    App_bootloader_check_meta_data();

    // 3.写入程序
    // 解锁flash
    HAL_FLASH_Unlock();
    // 3.1擦除足够的flash区域
    App_flash_erase((app_size / FLASH_PAGE_SIZE) + 1);
    // 3.2读出1页的内容
    // 3.3写入到flash中
    // 剩余程序的大小
    uint32_t app_size_left = app_size;
    uint16_t data_tmp = 0;
    uint32_t write_data_size;

    while (app_size_left >= FLASH_PAGE_SIZE)
    {
        //已经写入的数据大小
        write_data_size = app_size - app_size_left;
        //程序剩下的大小还大于1页 => 调用的地址是w25q128的地址
        Int_w25q128_read_data_with_32addr(app_start_addr + app_size - app_size_left, flash_data_buff, FLASH_PAGE_SIZE);
        app_size_left -= FLASH_PAGE_SIZE;
        //3.4写入1页数据到flash中
        for(uint16_t i = 0; i < FLASH_PAGE_SIZE; i += 2)
        {
            if(i + 1 < FLASH_PAGE_SIZE)
            {
                data_tmp = flash_data_buff[i] | flash_data_buff[i + 1] << 8;
                //写入到flash的地址
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_START + write_data_size + i, data_tmp);
            }
        }
    }
    //写入最后一页
    if(app_size_left > 0)
    {
        write_data_size = app_size - app_size_left;
        //3.5读出w25q128中剩下的程序
        Int_w25q128_read_data_with_32addr(app_start_addr + write_data_size, flash_data_buff, app_size_left);
        //3.6 将剩余的程序写入到最后一页flash中
        for (uint16_t i = 0; i < app_size_left; i += 2)
        {
            if (i + 1 < app_size_left)
            {
                data_tmp = flash_data_buff[i] | flash_data_buff[i + 1] << 8;
                // 写入到flash的地址
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_START + write_data_size + i, data_tmp);
            }
        }
    }

    // 2.上锁flash
    HAL_FLASH_Lock();
}

/**
 * @brief 执行更新操作
 *
 */
void App_bootloader_update()
{
    if (app_boot_update_status == BOOT_UPDATE)
    {
        // 将W25Q128中的程序写入到flash中
        // TODO:将W25Q128中的程序写入到flash中
        printf("app update\r\n");
        app_bootloader_write_app_flash();
    }
    else if (app_boot_update_status == BOOT_NO_UPDATE)
    {
        printf("no update\r\n");
    }
    else if (app_boot_update_status == BOOT_RESET)
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
    // 不管更新与否，最后都需要执行的跳转操作 到A程序中
    if (app_boot_update_status == BOOT_RESET)
    {
        // 跳转到出厂设置的默认程序  0x0800 4000
        Int_bootloader_jump_to_app(RESET_START);
    }
    else
    {
        // 不需要恢复出厂设置，直接跳转到A程序中  0x0800 8000
        Int_bootloader_jump_to_app(APP_START);
    }
}
