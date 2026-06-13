#include "Int_can.h"

/**
 * @brief 配置白名单过滤器  手动开启CAN
 *
 *
 */
void Int_CAN_init(void)
{

    CAN_FilterTypeDef filterConfig = {0};
    filterConfig.FilterBank = 0;                      // 选择过滤器编号 0-13 一共14个
    filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;  // 选择使用掩码模式 模糊匹配
    filterConfig.FilterScale = CAN_FILTERSCALE_32BIT; // 选择使用的位数   32位

    /**
     * ==================================================================================================
     * STM32 CAN 过滤器 - 32位屏蔽位模式寄存器对齐表 (严格列对齐版)
     * ==================================================================================================
     * 字节范围 |      [31:24] 字节      |          [23:16] 字节          |      [15:8] 字节       |       [7:0] 字节        |
     * ---------+------------------------+--------------------------------+------------------------+-------------------------+
     * ID寄存器 | CAN_FxR1[31:24]        | CAN_FxR1[23:16]                | CAN_FxR1[15:8]         | CAN_FxR1[7:0]           |
     * 屏蔽寄存 | CAN_FxR2[31:24]        | CAN_FxR2[23:16]                | CAN_FxR2[15:8]         | CAN_FxR2[7:0]           |
     * ---------+------------------------+--------------------------------+------------------------+-------------------------+
     * 实际映像 | STID[10:3]             | STID[2:0]   | EXID[17:13]      | EXID[12:5]             | EXID[4:0] | IDE | RTR | 0 |
     * ==================================================================================================
     * * 🛠️ 对应的寄存器位移计算（32位一体拼接）：
     * | 31                 21 | 20                         3 | 2       | 1       | 0     |
     * |-----------------------|------------------------------|---------|---------|-------|
     * |   STID[10:0] 标准ID   |       EXID[17:0] 扩展ID      |   IDE   |   RTR   | 固定0 |
     * |     (左移 21 位)      |         (左移 3 位)          | 扩展帧1 | 远程帧1 |       |
     * * 📌 过滤寄存器常用赋值公式：
     * - FilterIdHigh    = ((StdId << 21) | (ExtId >> 11)) >> 16;  // 高16位
     * - FilterIdLow     = (((StdId << 21) | (ExtId >> 11)) & 0xFFFF) | (IDE << 2) | (RTR << 1); // 低16位
     */

    // 填写寄存器 => 只要id为1的数据
    filterConfig.FilterIdHigh = 0x0020;    
    filterConfig.FilterIdLow = 0x0000;

    // 填写掩码 只要掩码都是0 表示ID不需要匹配上 掩码哪一位是1 表示哪一位需要匹配上
    filterConfig.FilterMaskIdHigh = 0xFffe0;
    filterConfig.FilterMaskIdLow = 0x0000;

    // 使用哪一个接收队列
    filterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; // 选择FIFO0
    //  使能过滤器
    filterConfig.FilterActivation = ENABLE;

    HAL_CAN_ConfigFilter(&hcan, &filterConfig);

    // 2.手动开启CAN
    HAL_CAN_Start(&hcan);
}

/**
 * @brief 发送消息
 * @param id 消息ID
 * @param len 数据长度
 * @param data 数据指针
 *
 */
void Int_CAN_send(uint16_t id, uint8_t *data, uint8_t len)
{
    // 等待发送邮箱空闲
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0)
        ;

    // 将发送的消息添加到发送邮箱
    CAN_TxHeaderTypeDef txHeader = {0};
    txHeader.IDE = CAN_ID_STD;   // 标准格式
    txHeader.StdId = id;         // 选择标准ID
    txHeader.DLC = len;          // 数据长度
    txHeader.RTR = CAN_RTR_DATA; // 数据帧
    uint32_t mailbox = 0;
    // 1.句柄 CAN编号    2.CAN消息的头信息    3.数据指针    4.邮箱编号
    HAL_CAN_AddTxMessage(&hcan, &txHeader, data, &mailbox);
}

/**
 * @brief 接收消息
 * @param rec_msg: 数组  最多一次可以获取3条消息
 * @param msg_count: 接收消息数量计数指针
 *
 */
void Int_CAN_receive_msg(CAN_Rec_MSG *rec_msg, uint8_t *msg_count)
{
    // 接收消息
    *msg_count = HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0);
    for (uint8_t i = 0; i < *msg_count; i++)
    {
        //指向当前使用的缓存
        CAN_Rec_MSG *rec_msg_tmp = &rec_msg[i];
        // 清空对应消息缓存
        memset(&rec_msg[i], 0, sizeof(CAN_Rec_MSG));
        // 读取一条消息
        HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &(rec_msg_tmp->txHeader), rec_msg_tmp->data);
    }
}
