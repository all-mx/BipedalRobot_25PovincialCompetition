#ifndef _ROBOT_RUN_H_
#define _ROBOT_RUN_H_

extern bool g_SensorControl; //传感器控制标志位


void TaskRobotRun(void);
void FullActRun(uint8 actFullnum,uint32 times);//��ʼ���������µĶ���
void FullActStop(void);




#endif

