#include "include.h"

bool fRobotRun = FALSE;//�����б�־λ
uint8 ActFullNum = 0;//��������
uint32 ActFullRunTimesSum = 1;//��ֵΪ0�������޴�

uint32 ActFullRunTimes = 0;//��ǰ�Ѿ����еĴ���

uint32 TimeActionRunTotal = 0;//����ʱ���ܼ�
bool fFrameRunFinish = TRUE;//ÿ֡������ϱ�־λ

uint8 FrameIndexSum = 0;//һ�������������ܹ��ж��ٶ���
uint8 FrameIndex = 0;//����������ĳһ�����ı�ţ���0��ʼ


void FullActRun(uint8 actFullnum,uint32 times)//��ʼ���������µĶ���
{
	uint8 frameIndexSum;
	FlashRead(MEM_FRAME_INDEX_SUM_BASE + actFullnum,1, &frameIndexSum);
	if(frameIndexSum > 0)//�ö�����Ķ���������0��˵������Ч�ģ��Ѿ����ع������ˡ�
	{
		FrameIndexSum = frameIndexSum;
		if(ActFullNum != actFullnum)
		{
			if(actFullnum == 0)
			{//0�Ŷ�����ǿ�����У������жϵ�ǰ�������е�����������
				fRobotRun = FALSE;
				ActFullRunTimes = 0;
				fFrameRunFinish = TRUE;
			}
		}
		else
		{	//ֻ��ǰ�����ζ���������ͬ�����޸Ĵ���
			ActFullRunTimesSum = times;
		}
		
		
		if(FALSE == fRobotRun)
		{
			ActFullNum = actFullnum;
			ActFullRunTimesSum = times;
			FrameIndex = 0;
			ActFullRunTimes = 0;
			fRobotRun = TRUE;
			
			TimeActionRunTotal = gSystemTickCount;
		}
		
	}
	

}
void FullActStop(void)
{
	fRobotRun = FALSE;
	ActFullRunTimes = 0;

	fFrameRunFinish = TRUE;

	FrameIndex = 0;
}



uint16 ActSubFrameRun(uint8 fullActNum,uint8 frameIndex)
{
	uint32 i = 0;

//	uint16 frameSumSum = 0;	//�����Ӷ�������������ŵģ��Ӷ�����֡�����ǲ�ȷ������
	//������Ҫ����һ���㡣����ǰ���Ӷ�����֡������
	uint8 frame[ACT_SUB_FRAME_SIZE];
	uint8 servoCount;
	uint32 time;
	uint8 id;
	uint16 pos;

	FlashRead((MEM_ACT_FULL_BASE) + (fullActNum * ACT_FULL_SIZE) + (frameIndex * ACT_SUB_FRAME_SIZE)
		,ACT_SUB_FRAME_SIZE,frame);
	
	servoCount = frame[0];
	time = frame[1] + (frame[2]<<8);

	if(servoCount > 8)
	{//���������8����˵�������˴�����
		FullActStop();
		return 0;
	}
	for(i = 0; i < servoCount; i++)
	{
		id =  frame[3 + i * 3];
		pos = frame[4 + i * 3] + (frame[5 + i * 3]<<8);
		ServoSetPluseAndTime(id,pos,time);
		BusServoCtrl(id,SERVO_MOVE_TIME_WRITE,pos,time);
	}
	return time;
}

//int a = 0;//直走是0，左转是-1，右转是1
// void TaskRobotRun(void)
// {

// 	if(fRobotRun)
// 	{
// 		if(TRUE == fFrameRunFinish)
// 		{//������ɺ�ʼ��һ֡��������
// 			fFrameRunFinish = FALSE;
// 			TimeActionRunTotal += ActSubFrameRun(ActFullNum,FrameIndex);//����֡������ʱ���ۼ���
// 		}
// 		else
// 		{
// 			if(gSystemTickCount >= TimeActionRunTotal)
// 			{//���ϼ����֡������ָ��ʱ�����������
// 				fFrameRunFinish = TRUE;
// 				if(++FrameIndex >= FrameIndexSum)
// 				{//��������ö��������һ������
// 					FrameIndex = 0;
// 					if(ActFullRunTimesSum != 0)
// 					{//������д�������0�����������޴����У��Ͳ�����if��䣬��һֱ������
// 						if(++ActFullRunTimes >= ActFullRunTimesSum)
// 						{//�������д���������ֹͣ
// 							McuToPCSendData(CMD_FULL_ACTION_STOP,0,0);
// 							fRobotRun = FALSE;
// 							if(ActFullNum == 100)
// 							{
// 								FullActRun(101,1);
// 							}
// 							else if (ActFullNum == 101)
// 							{
// 								FullActRun(102,1);
// 							}
// 							else if (ActFullNum == 102)
// 							{
// 								FullActRun(103,1);
// 							}
// 							else if (ActFullNum == 103)
// 							{
// 								FullActRun(104,1);
// 							}
// 							else if (ActFullNum == 104)
// 							{
// 								FullActRun(105,1);
// 							}
// 							else if (ActFullNum == 105)
// 							{
// 								FullActRun(106,1);
// 							}
// 							else if (ActFullNum == 106)
// 							{
// 								FullActRun(107,1);
// 							}
// 							else if (ActFullNum == 107)
// 							{
// 								FullActRun(108,1);
// 							}
// 							else if (ActFullNum == 108)
// 							{
// 								FullActRun(109,1);
// 							}
// 							else if (ActFullNum == 109)
// 							{
// 								FullActRun(110,1);
// 							}
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// 	else
// 	{
// 		FrameIndex = 0;
		
// 		ActFullRunTimes = 0;

// 		fFrameRunFinish = TRUE;

// 		TimeActionRunTotal = gSystemTickCount;
// 		//ֻ��Ҫ������������������ʼ��һ�³�ֵ�Ϳ��ԣ������ۻ����
// 	}
// }

// 定义全局变量a（根据实际情况放在合适的头文件或全局区域）
int a = 0;  // 2:运行100, -1:运行101, 1:运行102, 3：复位，4：运行104（右脚先迈）其他值:不触发

void TaskRobotRun(void)
{
    // 事件触发逻辑：检测a的值并启动对应动作组
    static int last_a = 0;  // 记录上一次a的值，用于检测变化
    if (a != last_a)
    {
        // 停止当前正在运行的动作（如果有）
        fRobotRun = FALSE;
        // 重置状态变量，确保新动作组从头开始
        FrameIndex = 0;
        ActFullRunTimes = 0;
        fFrameRunFinish = TRUE;
        TimeActionRunTotal = gSystemTickCount;

        // 根据a的值启动对应动作组
        switch(a)
        {
            case 2:
                FullActRun(100, 0);  // 运行动作组100
                break;
            case -1:
                FullActRun(101, 0);  // 运行动作组101
                break;
            case 1:
                FullActRun(102, 0);  // 运行动作组102
                break;
						case 3:
								FullActRun(103, 0); 
						    break;
						case 4:
						    FullActRun(104, 0);
								break;
            default:
                // 其他值不触发任何动作（保持停止状态）
                return;
        }
        // 启动动作运行总开关
        fRobotRun = TRUE;
        // 更新上一次a的值，避免重复触发
        last_a = a;
    }

    // 原有动作运行逻辑（保持不变，仅执行当前启动的动作组）
    if(fRobotRun)
    {
        if(TRUE == fFrameRunFinish)
        {
            fFrameRunFinish = FALSE;
            TimeActionRunTotal += ActSubFrameRun(ActFullNum, FrameIndex);
        }
        else
        {
            if(gSystemTickCount >= TimeActionRunTotal)
            {
                fFrameRunFinish = TRUE;
                if(++FrameIndex >= FrameIndexSum)
                {
                    FrameIndex = 0;
                    if(ActFullRunTimesSum != 0)
                    {
                        if(++ActFullRunTimes >= ActFullRunTimesSum)
                        {
                            // 动作组运行结束，发送停止指令
                            McuToPCSendData(CMD_FULL_ACTION_STOP,0,0);
                            fRobotRun = FALSE;
                            // 清除触发状态（可选：根据需求是否保留a的值）
                            // a = 2;  // 若需要运行后重置a，可取消注释
                        }
                    }
                }
            }
        }
    }
    else
    {
        // 停止状态下的重置逻辑（保持不变）
        FrameIndex = 0;
        ActFullRunTimes = 0;
        fFrameRunFinish = TRUE;
        TimeActionRunTotal = gSystemTickCount;
    }
}