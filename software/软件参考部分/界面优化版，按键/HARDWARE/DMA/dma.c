#include "dma.h"
#include "usart.h"

/******************************************************************
��������:MYDMA1_Config()
��������:DMA1��ʼ������
����˵��:DMA_CHx��DMAͨ��ѡ��
		 cpar��DMA����ADC����ַ
		 cmar��DMA�ڴ����ַ
		 cndtrDMAͨ����DMA����Ĵ�С
��    ע:
*******************************************************************/
void MYDMA1_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
	
    DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA����ADC����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴������ȡ���͵��ڴ�//
	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ�� xӵ�и����ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA_CHx, &DMA_InitStructure);  //ADC1ƥ��DMAͨ��1
	
	DMA_ITConfig(DMA1_Channel1,DMA1_IT_TC1,ENABLE);	//ʹ��DMA�����ж�	
	
	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	

	DMA_Cmd(DMA1_Channel1,ENABLE);//ʹ��DMAͨ��
}

/******************************************************************
	DMA�жϺ���
*******************************************************************/
//void  DMA1_Channel1_IRQHandler(void)
//{
//	if(DMA_GetITStatus(DMA1_IT_TC1)!=RESET){
//		//�жϴ������
//    	printf("The current value =%d \r\n",ADC_ConvertedValue);
//		DMA_ClearITPendingBit(DMA1_IT_TC1);
//	}
//}

