#include "app_update.h"

Update_State_t update_state = UPDATE_IDLE;

    static void
    App_run(void)
{
    //2.0程序改成闪烁
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    HAL_Delay(1000);
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
    default:
        break;
    }
}
