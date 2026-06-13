#ifndef __INT_CAN_H__
#define __INT_CAN_H__

#include "can.h"
#include "string.h"

typedef struct
{
    CAN_RxHeaderTypeDef txHeader;
    uint8_t data[8];
} CAN_Rec_MSG;

/**
 * @brief 配置白名单过滤器  手动开启CAN
 * 
 *
 */
void Int_CAN_init(void);


/**
 * @brief 发送消息
 * @param id 消息ID
 * @param len 数据长度
 * @param data 数据指针
 * 
 */
void Int_CAN_send(uint16_t id, uint8_t *data, uint8_t len);

/**
 * @brief 接收消息
 * @param rec_msg: 数组  最多一次可以获取3条消息
 * @param msg_count: 接收消息数量计数指针
 *
 */
void Int_CAN_receive_msg(CAN_Rec_MSG *rec_msg, uint8_t *msg_count);

#endif
