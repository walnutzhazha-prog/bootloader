#ifndef __APP_UPDATE_H__
#define __APP_UPDATE_H__

#include "Int_can.h"
#include "usart.h"
#include "Int_bootloader.h"
#include "crc.h"



#define APP_UPDATE_CMD_ID   1
#define APP_UPDATE_CMD_1 'h'
#define APP_UPDATE_CMD_2 'j'
#define APP_UPDATE_CMD_3 'c'

typedef enum
{
    APP_UPDATE_CMD_WAIT = 0,
    APP_UPDATE_SEND_APP,
}APP_UPDATE_STATE_E;


/**
 * @brief 初始化上位机更新程序
 * 
 */
void App_update_init(void);


/**
 * @brief 等待开发板发送更新请求
 * 
 */
void App_update_wait_cmd(void);


/**
 * @brief 发送应用程序到开发板
 * 
 * 
 */
void App_update_send_app(void);


/**
 * @brief 循环程序
 * * 
 * 
 */
void App_update_work(void);

#endif
