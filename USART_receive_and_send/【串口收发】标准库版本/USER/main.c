#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"

#define FRAME_HEADER 0x01
#define FRAME_TAIL 0x02

int receive_flag = 0; //���ڽ�����ɱ�־λ  0��δ���  1�������
u8 Receive_Data[8];
void uart_init(u32 bound)
{
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 
}


int main(void)
{	
	delay_init();	    //��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200	
	
//	USART_SendData(USART1, 0x11);
//	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���

	while(1)
	{
		if(receive_flag == 1)
		{
				for(int i=0;i<8;i++)
				{
					USART_SendData(USART1, Receive_Data[i]);//�򴮿�1��������
					while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���				
				}	
				receive_flag = 0;			
		}
	}		
}


void USART1_IRQHandler(void)
{	
	static u8 Count=0;  //���������̬����
	u8 Usart_Receive;   //�����м䴫�ݱ���

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //�ж��Ƿ���յ�����
	{
			Usart_Receive = USART_ReceiveData(USART1); //��ȡ����
		
			if(receive_flag == 0)
			{
					//����������������
					Receive_Data[Count]=Usart_Receive;
					
					//ȷ�������һ������ΪFRAME_HEADER��֡ͷ��
					if(Usart_Receive == FRAME_HEADER||Count>0) 
						Count++; 
					else 
						Count=0;
					
					if (Count == 8) //��֤���ݰ��ĳ���
					{   
						Count=0; //Ϊ����������������������׼��
						
						if(Receive_Data[7] == FRAME_TAIL) //��֤���ݰ���֡β
						{
							//���յ�������һ֡����֮������������д���
							receive_flag = 1;												
						}
					}					
			}			
	}
}




