#ifndef __APP_UPDATE_H__
#define __APP_UPDATE_H__


#include "usart.h"
#include "Int_can.h"
#include "crc.h"
#include "Int_w25q128.h"
#include "Int_w24c02.h"

#define BOOTLOADER_UART_REC_BUFF_LEN 32

#define APP_UPDATE_CMD_ID 0
//更新指令
#define APP_UPDATE_CMD "hjc"
#define APP_UPDATE_CMD_LEN 3

//存储更新状态的地址
#define CHECK_UPDATE_ADDR   0x10
//更新状态的值
#define BOOT_UPDATE         0x01
#define BOOT_NO_UPDATE      0x02
//添加校验的密钥
#define CHECK_KEY_ADDR      0x11
#define CHECK_KEY           0x5A6B

//16K缓冲区 接收整个程序
#define APP_DATA_MAX_LEN 16384

//w25q128 存放元数据的地址  存放到第0扇
#define FLASH_META_ADDR 0x000000
//w25q128 存放程序的地址    存放到第1扇
#define FLASH_APP_ADDR  0x001000


//程序状态机
typedef enum
{
    UPDATE_IDLE = 0,
    UPDATE_RECV_SEND_CMD,
    UPDATE_RECV_DATA,
    UPDATE_RECV_CHECK_DATA,
    UPDATE_RECV_BOOT_UPDATE,
    UPDATE_END
}Update_State_t;

/**
 * @brief 接收串口数据 => 收到更新标记 => 发送CAN的更新指令
 * 
 * @param None
 * @return None
 */
void App_update_init(void);



/**
 * @brief 使用CAN发送更新指令
 * 
 * @param None
 * @return None
 */
void App_update_send_update_cmd(void);


/**
 * @brief can接收应用数据   保存到W25Q128中
 * 
 * @param None
 * @return None
 */
void App_update_receive_app_data(void);

/**
 * @brief 修改在w24c02中的更新标志位
 *
 * @param None
 * @return None
 */
void App_update_change_boot_mode(void);


/**
 * @brief 循环调用  执行状态机逻辑
 * 
 * @param None
 * @return None
 */
void App_update_work(void);

#endif // __APP_UPDATE_H__
