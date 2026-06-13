#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

void send_bin_file(const char *file_path, int sock_fd)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        printf("打开bin文件失败!\n");
        return;
    }

    struct can_frame frame;
    frame.can_id = 0x123; // 与单片机过滤器匹配的ID
    frame.can_dlc = 8;

    uint8_t packet_idx = 0;
    uint8_t buffer[6];
    size_t read_len;

    // 1. 发送开始指令 (Type: 0x01)
    frame.data[0] = 0x01;
    frame.data[1] = packet_idx++;
    // 可以把文件总大小写进剩余字节里告诉单片机
    write(sock_fd, &frame, sizeof(struct can_frame));
    usleep(5000); // 留给单片机准备擦除 Flash 的时间

    // 2. 循环读取并分包发送 (Type: 0x02)
    while ((read_len = fread(buffer, 1, 6, file)) > 0)
    {
        memset(frame.data, 0, 8);
        frame.data[0] = 0x02;         // 数据传输状态
        frame.data[1] = packet_idx++; // 包序号
        memcpy(&frame.data[2], buffer, read_len);
        frame.can_dlc = read_len + 2; // 实际控制DLC大小

        write(sock_fd, &frame, sizeof(struct can_frame));

        // 关键点：每发一帧，必须等单片机回复一个“应答(ACK)”帧再发下一帧！
        // 如果盲发，Linux发得太快，单片机来不及写 Flash 就会导致丢包卡死。
        wait_for_stm32_ack(sock_fd);
    }

    // 3. 发送结束指令 (Type: 0x03)
    frame.data[0] = 0x03;
    frame.can_dlc = 2;
    write(sock_fd, &frame, sizeof(struct can_frame));

    fclose(file);
    printf("固件发送完成!\n");
}