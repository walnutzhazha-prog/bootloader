#include "app_update.h"

uint8_t uart_rec_buff[32] = {0};

Update_State_t update_state = UPDATE_IDLE;

// 声明一个能够容纳整个程序的静态缓存    BSS段里面   SRAM空间
uint8_t app_data_buff[APP_DATA_MAX_LEN] = {0};

// CAN接收消息的缓冲区
CAN_Rec_MSG can_rec_msg[3] = {0};
// 单次接收消息的条数
uint8_t can_rec_msg_cnt = 0;
// 接收程序的长度
uint16_t can_rec_msg_len = 0;
// 记录当前一次接收到时间
uint32_t can_rec_time = 0;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    (void)Size;
    // 串口接收到了数据  -> 如果是cmd
    if ((huart->Instance == USART1) && (update_state == UPDATE_IDLE))
    {
        // 校验数据 => cmd => 让开发板给上位机发送更新指令  使用CAN发送
        if (strstr((char *)uart_rec_buff, "cmd"))
        {
            update_state = UPDATE_RECV_SEND_CMD;
            // 添加回滚逻辑 => 如果校验失败 还能重新接收cmd
            //  清空掉初始化串口使用之前的所有问题
            __HAL_UART_CLEAR_OREFLAG(&huart1);
            __HAL_UART_CLEAR_IDLEFLAG(&huart1);
            // 带有中断的串口接收函数
            // 少一个参数 => 超时时间   因为IT带中断的函数方法是异步执行的
            HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart_rec_buff, BOOTLOADER_UART_REC_BUFF_LEN);
        }
    }
}

static void App_run(void)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
    HAL_Delay(200);
}

/**
 * @brief 接收串口数据 => 收到更新标记 => 发送CAN的更新指令
 *
 * @param None
 * @return None
 */
void App_update_init(void)
{
    // 启动串口的接收程序
    // 清空掉初始化串口使用之前的所有问题
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    // 带有中断的串口接收函数
    // 少一个参数 => 超时时间   因为IT带中断的函数方法是异步执行的
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart_rec_buff, BOOTLOADER_UART_REC_BUFF_LEN);
    Int_CAN_init();
}

/**
 * @brief 使用CAN发送更新指令
 *
 * @param None
 * @return None
 */
void App_update_send_update_cmd(void)
{
    Int_CAN_send(APP_UPDATE_CMD_ID, (uint8_t *)APP_UPDATE_CMD, APP_UPDATE_CMD_LEN);
    // 更新命令已经发送  修改状态为接收程序
    update_state = UPDATE_RECV_DATA;
}

/**
 * @brief can接收应用数据   保存到W25Q128中
 *
 * @param None
 * @return None
 */
void App_update_receive_app_data(void)
{
    Int_CAN_receive_msg(can_rec_msg, &can_rec_msg_cnt);
    for (uint8_t i = 0; i < can_rec_msg_cnt; i++)
    {
        can_rec_time = HAL_GetTick();
        // 将数据缓存到app_data_buff中
        memcpy(app_data_buff + can_rec_msg_len, can_rec_msg[i].data, can_rec_msg[i].txHeader.DLC);
        // 记录接收程序的长度
        can_rec_msg_len += can_rec_msg[i].txHeader.DLC;
    }
    can_rec_msg_cnt = 0;

    // 判断接收完成
    if (can_rec_time != 0 && can_rec_time + 2000 < HAL_GetTick())
    {
        // 已经断开发送数据2s
        printf("recv data len: %d\r\n", can_rec_msg_len);
        update_state = UPDATE_RECV_CHECK_DATA;
    }
}

static uint32_t App_crc_cal(uint8_t *data, uint16_t len)
{
    uint32_t *p_data = (uint32_t *)data;
    uint32_t word_count = (len + 3) / 4;

    // 复位CRC
    __HAL_CRC_DR_RESET(&hcrc);

    uint32_t crc_val = HAL_CRC_Calculate(&hcrc, p_data, word_count);
    return crc_val;
}

/**
 * @brief 校验接收数据的CRC值
 *
 * @param None
 * @return None
 */
void App_update_check_data(void)
{
    Int_CAN_receive_msg(can_rec_msg, &can_rec_msg_cnt);
    for (uint8_t i = 0; i < can_rec_msg_cnt; i++)
    {
        // 读取发送过来的CRC值   发送CRC值 4字节 低位在前
        uint32_t rec_crc_val = can_rec_msg[i].data[0] | (can_rec_msg[i].data[1] << 8) | (can_rec_msg[i].data[2] << 16) | (can_rec_msg[i].data[3] << 24);
        uint32_t crc_val_calc = App_crc_cal(app_data_buff, can_rec_msg_len);
        if (rec_crc_val == crc_val_calc)
        {
            printf("crc check pass\r\n");
            update_state = UPDATE_RECV_BOOT_UPDATE;
        }
        else
        {
            // 校验失败 => 回滚到idle状态
            printf("crc check fail\r\n");
            // 清空缓存和状态
            memset(app_data_buff, 0, APP_DATA_MAX_LEN);
            can_rec_msg_len = 0;
            can_rec_time = 0;
            // 回滚到发送更新指令状态
            update_state = UPDATE_IDLE;
        }
    }
}

uint8_t w25q128_write_buff[8];

/**
 * @brief 修改在w24c02中的更新标志位
 *
 * @param None
 * @return None
 */
void App_update_change_boot_mode(void)
{
    // 1.将程序写入到外置flash w25q128中
    // 1.1 擦除正确的区域和足够的空间
    uint16_t sector_earse_count = can_rec_msg_len / 4096 + 2;
    for (uint8_t i = 0; i < sector_earse_count; i++)
    {
        Int_w25q128_erase_sector(0, i);
    }

    // 1.2 写入元数据
    w25q128_write_buff[0] = FLASH_APP_ADDR & 0xff;
    w25q128_write_buff[1] = (FLASH_APP_ADDR >> 8) & 0xff;
    w25q128_write_buff[2] = (FLASH_APP_ADDR >> 16) & 0xff;
    w25q128_write_buff[3] = (FLASH_APP_ADDR >> 24) & 0xff;
    w25q128_write_buff[4] = can_rec_msg_len & 0xff;
    w25q128_write_buff[5] = (can_rec_msg_len >> 8) & 0xff;
    w25q128_write_buff[6] = (can_rec_msg_len >> 16) & 0xff;
    w25q128_write_buff[7] = (can_rec_msg_len >> 24) & 0xff;
    Int_w25q128_write_data_with_32addr(FLASH_META_ADDR, w25q128_write_buff, 8);
    uint16_t write_len = 0;
    uint16_t write_tmp_len = 0;

    // 1.3按照页 将程序写入到w25q128中
    while (write_len < can_rec_msg_len)
    {
        // 剩下的长度是否超过一页
        if (can_rec_msg_len - write_len >= 256)
        {
            write_tmp_len = 256;
        }
        else
        {
            write_tmp_len = can_rec_msg_len - write_len;
        }
        Int_w25q128_write_data_with_32addr(FLASH_APP_ADDR + write_len, app_data_buff + write_len, write_tmp_len);
        write_len += write_tmp_len;
    }

    // 2.修改w24c02中的更新标志位   密钥高8位在前
    uint8_t eeprom_buff[3] = {0};
    eeprom_buff[0] = BOOT_UPDATE;
    eeprom_buff[1] = (CHECK_KEY >> 8) & 0xff;
    eeprom_buff[2] = CHECK_KEY & 0xff;

    Int_w24c02_write_bytes(CHECK_UPDATE_ADDR, eeprom_buff, 3);
    HAL_Delay(10);
    printf("boot update success\r\n");

    // 修改状态
    update_state = UPDATE_END;
}


/**
 * @brief 循环调用  执行状态机逻辑
 *
 * @param None
 * @return None
 */
void App_update_work(void)
{
    switch (update_state)
    {
    case UPDATE_IDLE:
        // 只有不需要更新程序的时候  才会运行程序之前的功能
        App_run();
        break;
    case UPDATE_RECV_SEND_CMD:
        printf("recv cmd\r\n");
        printf("send cmd\r\n");
        App_update_send_update_cmd();
        // 在接收数据之前 清空缓冲区
        memset(app_data_buff, 0, APP_DATA_MAX_LEN);
        can_rec_msg_len = 0;
        break;
    case UPDATE_RECV_DATA:
        App_update_receive_app_data();
        break;
    case UPDATE_RECV_CHECK_DATA:
        App_update_check_data();
        break;
    case UPDATE_RECV_BOOT_UPDATE:
        App_update_change_boot_mode();
        break;
    case UPDATE_END:
        // 延时 => 重启程序
        HAL_Delay(1000);
        // 重启程序
        HAL_NVIC_SystemReset();
        break;
    default:
        break;
    }
}
