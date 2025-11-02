/* SensorIO.c
 * 简化实现：PB4 = DRDY(外部中断, EXTI4), PB5..PB9 = DATA0..DATA4 (输入，带上拉)
 * 请根据实际硬件修改 SENSOR_GPIO_PORT/SENSOR_PIN_* 宏。
 */

#include "include.h"
#include "SensorIO.h"

// 可修改的引脚映射（默认示例）
#define SENSOR_GPIO_PORT        GPIOB
#define SENSOR_RCC_APB2         RCC_APB2Periph_GPIOB

#define SENSOR_PIN_DRDY         GPIO_Pin_4
#define SENSOR_PIN_D0           GPIO_Pin_5
#define SENSOR_PIN_D1           GPIO_Pin_6
#define SENSOR_PIN_D2           GPIO_Pin_7
#define SENSOR_PIN_D3           GPIO_Pin_8
#define SENSOR_PIN_D4           GPIO_Pin_9

static volatile bool s_packetReady = FALSE;
static uint8 s_packet[7]; // [0]=start, [1]=cmd, [2..6]=data0..data4

void InitSensorIO(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 时钟
    RCC_APB2PeriphClockCmd(SENSOR_RCC_APB2 | RCC_APB2Periph_AFIO, ENABLE);

    // 数据引脚：上拉输入
    GPIO_InitStructure.GPIO_Pin = SENSOR_PIN_D0 | SENSOR_PIN_D1 | SENSOR_PIN_D2 | SENSOR_PIN_D3 | SENSOR_PIN_D4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SENSOR_GPIO_PORT, &GPIO_InitStructure);

    // DRDY 引脚：上拉输入，连接 EXTI
    GPIO_InitStructure.GPIO_Pin = SENSOR_PIN_DRDY;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(SENSOR_GPIO_PORT, &GPIO_InitStructure);

    // 将 PB4 连接到 EXTI4
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);

    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 假设 DRDY 下降沿有效
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // NVIC
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 初始化内部包为 0
    s_packet[0] = 0xAA;
}

// EXTI4 IRQ Handler
void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        // 读取 5 路数据（按位读取，1 表示高电平，由硬件决定含义）
        uint8 d0 = GPIO_ReadInputDataBit(SENSOR_GPIO_PORT, SENSOR_PIN_D0) ? 1 : 0;
        uint8 d1 = GPIO_ReadInputDataBit(SENSOR_GPIO_PORT, SENSOR_PIN_D1) ? 1 : 0;
        uint8 d2 = GPIO_ReadInputDataBit(SENSOR_GPIO_PORT, SENSOR_PIN_D2) ? 1 : 0;
        uint8 d3 = GPIO_ReadInputDataBit(SENSOR_GPIO_PORT, SENSOR_PIN_D3) ? 1 : 0;
        uint8 d4 = GPIO_ReadInputDataBit(SENSOR_GPIO_PORT, SENSOR_PIN_D4) ? 1 : 0;

        s_packet[0] = 0xAA; // start
        s_packet[1] = 0x10; // 默认 cmd: 0x10 表示直接映射为动作/舵机控制（可自定义）
        s_packet[2] = d0;
        s_packet[3] = d1;
        s_packet[4] = d2;
        s_packet[5] = d3;
        s_packet[6] = d4;

        s_packetReady = TRUE;

        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

bool Sensor_PacketReady(void)
{
    return s_packetReady;
}

// 将数据包解析并触发对应动作（在主循环中调用以避免长时间 ISR）
void Sensor_ProcessPacket(void)
{
    if(! g_SensorControl)
    {
        return;

    }
    if(!s_packetReady) return;

    // 读取并清标志
    uint8 local[7];
    for(int i=0;i<7;i++) local[i] = s_packet[i];
    s_packetReady = FALSE;

    // 简单解析逻辑：
    // cmd == 0x10: 将 5 位并成 pattern（bit0 = data0），如果 pattern==0 则停止动作组，
    // 否则调用 FullActRun(100 + pattern, 1) 作为示例（请根据实际动作号映射调整）

    if(local[0] != 0xAA) return; // 非法包

    uint8 cmd = local[1];
    if(cmd == 0x10)
    {
        uint8 pattern = (local[2] & 0x01)
                        | ((local[3] & 0x01) << 1)
                        | ((local[4] & 0x01) << 2)
                        | ((local[5] & 0x01) << 3)
                        | ((local[6] & 0x01) << 4);

        if(pattern == 0)
        {
            FullActStop();          //代表全识别为黑线则静止
        }
        else
        {
            // 调用动作组示例：100 + pattern
            uint8 actId = 100 + pattern;
            FullActRun(actId, 1);
        }
    }
    else
    {
        // 其它命令可扩展，例如直接控制舵机姿态：
        // cmd == 0x11: local[2..6] => 五路模拟数值，映射到舵机 1..5
        if(cmd == 0x11)
        {
            // 将 0/1 映射为两个预置位置，实际项目中应替换为合适的量程/映射
            uint16 time = 500;
            for(uint8 i=0;i<5;i++)
            {
                uint8 val = local[2 + i];
                uint8 servoId = i + 1; // 映射为舵机 ID 1..5（请根据实际硬件修改）
                uint16 pos = val ? 800 : 400; // 示例位置值
                ServoSetPluseAndTime(servoId, pos, time);
                BusServoCtrl(servoId, SERVO_MOVE_TIME_WRITE, pos, time);
            }
        }
    }
}
