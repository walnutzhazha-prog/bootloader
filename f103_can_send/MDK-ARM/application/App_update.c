#include "App_update.h"

// 标记上位机当前的状态
APP_UPDATE_STATE_E g_app_update_state = APP_UPDATE_CMD_WAIT;
// 用于开发板发送更新请求的消息缓存
CAN_Rec_MSG msgs[3] = {0};
// 接收消息计数
uint8_t msg_count = 0;

// 记录程序的总长度
extern uint16_t uart_rec_total_len;
// 已经发送的数据长度
uint16_t update_data_len = 0;
// CAN发送数据的缓冲区
uint8_t update_buff[8] = {0};

/**
 * @brief 初始化上位机更新程序
 *
 */
void App_update_init(void)
{
    printf("App_update_init\r\n");
    g_app_update_state = APP_UPDATE_CMD_WAIT;
    Int_CAN_init();
    printf("App_update_wait_cmd\r\n");

    // 如果已经烧录好更新的程序  需要手动填写一下uart_rec_total_len
    if (uart_rec_total_len == 0)
    {
        uart_rec_total_len = 11104;
    }
}

/**
 * @brief 等待开发板发送更新请求
 *
 */
void App_update_wait_cmd(void)
{
    Int_CAN_receive_msg(msgs, &msg_count);
    // 只接收ID为0的消息
    for (uint8_t i = 0; i < msg_count; i++)
    {
        if (msgs[i].txHeader.StdId == 0)
        {
            // 判断是否为更新消息 => 'hjc'
            if (msgs[i].data[0] == APP_UPDATE_CMD_1 && msgs[i].data[1] == APP_UPDATE_CMD_2 && msgs[i].data[2] == APP_UPDATE_CMD_3)
            {
                g_app_update_state = APP_UPDATE_SEND_APP;
                printf("App_update_send_app\r\n");
                //可选 => 接收到更新指令之后  延时等待一下再发送程序
                HAL_Delay(100);
            }
        }
    }
}

static uint32_t App_crc_cal(uint32_t flash_addr, uint16_t len)
{
    //将flash地址转化为32位指针
    uint32_t *p_data = (uint32_t *)flash_addr;
    uint32_t word_count = (len + 3) / 4;

    // 复位CRC
    __HAL_CRC_DR_RESET(&hcrc);

    uint32_t crc_val = HAL_CRC_Calculate(&hcrc, p_data, word_count);
    return crc_val;
}

/**
 * @brief 发送应用程序到开发板
 *
 *
 */
void App_update_send_app(void)
{
    if (update_data_len < uart_rec_total_len)
    {
        uint8_t send_len = 0;
        // 剩下的长度够不够8字节
        if (uart_rec_total_len - update_data_len >= 8)
        {
            send_len = 8;
        }
        else
        {
            send_len = uart_rec_total_len - update_data_len;
        }
        for (uint8_t i = 0; i < send_len; i++)
        {
            update_buff[i] = *(volatile uint8_t *)(APP_START_ADDR + update_data_len + i);
        }
        update_data_len += send_len;
        // 发送字节   200Kbps
        Int_CAN_send(APP_UPDATE_CMD_ID, update_buff, send_len);

        //如果下位机接收处理的速度不够  上位机需要加延时
        // HAL_Delay(1);

        // 每次发送足够数量的字节 256字节  => 延时等待一下
        if (update_data_len % 256 == 0)
        {
            HAL_Delay(100);
        }
    }
    else
    {
        // 发送完成
        printf("App_update_send_app_finish\r\n");
        update_data_len = 0;
        g_app_update_state = APP_UPDATE_CMD_WAIT;

        // 发送CRC校验
        HAL_Delay(2100);
        uint32_t crc_value = App_crc_cal(APP_START_ADDR, uart_rec_total_len);
        memset(update_buff, 0, sizeof(update_buff));
        //将crc_value转换为4字节的数组 低位在前
        update_buff[0] = crc_value & 0xFF;
        update_buff[1] = (crc_value >> 8) & 0xFF;
        update_buff[2] = (crc_value >> 16) & 0xFF;
        update_buff[3] = (crc_value >> 24) & 0xFF;  
        Int_CAN_send(APP_UPDATE_CMD_ID, update_buff, 4);
    }
}

void App_update_work(void)
{
    switch (g_app_update_state)
    {
    case APP_UPDATE_CMD_WAIT:
        App_update_wait_cmd();
        break;
    case APP_UPDATE_SEND_APP:
        App_update_send_app();
        break;
    default:
        break;
    }
}
