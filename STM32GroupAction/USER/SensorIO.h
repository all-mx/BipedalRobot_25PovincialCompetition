/* SensorIO.h
 * 简单的 5 路 GPIO 传感器接收模块接口
 * 默认假设：PB4 = DataReady (外部中断)，PB5-PB9 = 5 路数据输入（每路 0/1）
 * 包格式（简化示例）：[0]=0xAA(start), [1]=cmd, [2..6]=5 路数据 (0/1)
 * 处理策略：在 DRDY 中断读取数据，设置就绪标志；主循环调用 Sensor_ProcessPacket() 进行解析并
 * 调用舵机/动作组接口（例如 FullActRun / FullActStop / ServoSetPluseAndTime）。
 *
 * 注意：这是一个示例实现，具体引脚与数据格式请根据实际硬件修改宏与解析逻辑。
 */

#ifndef __SENSORIO_H
#define __SENSORIO_H

void InitSensorIO(void);
// 主循环调用，处理最近接收到的数据包（非阻塞）
void Sensor_ProcessPacket(void);
// 查询是否有未处理的数据包
bool Sensor_PacketReady(void);

#endif // __SENSORIO_H
