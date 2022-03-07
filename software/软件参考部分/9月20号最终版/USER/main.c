#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "adc.h"
#include "dma.h"
#include "timer.h"
#include "beep.h"
#include "exti.h"
#include "math.h"
#include "table_fft.h"
#include "stm32_dsp.h"
 
/************************************************
stm32ʾ�������
PA4Ϊģ���ѹ�����
************************************************/

#define NPT  1024

void clear_point(u16 mode);
void GetPowerMag(void);

	
	int t,i;
	u32 adcx[NPT];//adc��ֵ����


	 u8 adc_flag=0;	//����������־
	 u8 key_flag=0;	//����ɨ���־
	 u8 show_flag=1;	//������ͣ��־
	 u32 adcmax;	//�������ֵ����Сֵ
	 u32 adcmin;
	 u32 currentadc ;	//ʵʱ��������
	 u16 T=2250;		//��ʱ��2����ֵ			(�޸�plus����)
	 u16 pre=1;			//��ʱ��2Ԥ��Ƶֵ
	 u32 fre;			//����Ƶ�� kHz
	 int choice=0;		//��������
	
	
//��FFTģʽ�µĵı���
	int long fftin [NPT];//FFT����
	int long fftout[NPT];//FFT���
	float FFT_Mag[NPT/2]={0};//��Ƶ����
	u16 magout[NPT];//ģ�����Ҳ����������

	float mag1;			//г��1�ķ�ֵ
	float TTT[5]={0};		//���THD������
	int th=0;
	float temp;
	float F;
	
	
    int main(void)
 {	 

	MYDMA1_Config(DMA1_Channel1,(u32)&ADC1->DR,(u32)&currentadc,1);
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
 	LED_Init();			     //LED�˿ڳ�ʼ��
	EXTIX_Init();			//�ⲿ�жϳ�ʼ��	
	BEEP_Init();			//��ʼ��
	LCD_Init();
	LED_Init();		//led��ʼ��
 	Adc_Init();		  		//ADC��ʼ��
	window();				//�����ʼ��
	TIM2_PWM_Init(T-1,pre-1);	//���Ƶ��72000000/1/1000=72KHz
	 
	 
	 
	 
       
	while(1)
	{
		LED0=0;
		//�ȴ��������
		
		while(adc_flag==0)
		{
			LED1=!LED1;
		}
		adc_flag=0;
		
		//��ȡ�����Сֵ
		adcmax=adcx[1];
		adcmin=adcx[1];
		for(i=0;i<NPT;i++)
		{
			fftin[i] = 0;
			fftin[i] = adcx[i] << 16;
			
			//���ѹ�����ֵ����Сֵ
			if(adcx[i] >= adcmax)
			{			
				adcmax = adcx[i];
			}			
			if(adcx[i] <= adcmin)
			{			
				adcmin = adcx[i];
			}
		}
			adcmax=adcmax*0.799;   //0.8 �� 3300/4096	
			adcmin=adcmin*3300/4096;
		
			POINT_COLOR=BLUE;
			LCD_ShowxNum(280,100,adcmax,4,16,0);//��ʾ��ѹ���ֵ
			LCD_ShowxNum(280,150,adcmin,4,16,0);//��ʾ��ѹ��Сֵ
			LCD_ShowxNum(280,200,adcmax-adcmin,4,16,0);//��ʾ��ѹ���ֵ
		
			GetPowerMag();
		
			LED0=!LED0;
			DMA_Cmd(DMA1_Channel1,ENABLE);//ʹ��DMA1-CH1
			delay_ms(10);
		
			if(show_flag==1)
			{
				clear_point(1);    //������ʾ����ǰ�У��������߻���
			}	
	}	



		

	}
 
	
//DMA�ж����ú�������ADC�ɼ�������ת�Ƶ�������	
	
	void DMA1_Channel1_IRQHandler(void) 
		{
			if(DMA_GetITStatus(DMA1_IT_TC1)!=RESET)
			{
				adcx[t]=currentadc;
				t++;
				if(t==NPT)
				{
					t=0;
					adc_flag=1;
					DMA_Cmd(DMA1_Channel1, DISABLE);        //ʧ��DMA
				}
			}
			DMA_ClearITPendingBit(DMA1_IT_TC1);		
		}
		
		
		
 /******************************************************************
��������:clear_point()
��������:ѭ�����²���
����˵��:mode ����ģʽѡ�� 1��������ģʽ��0�������ģʽ
��    ע:���ε���ʾ�ɲ��ô�㷽ʽ�ͻ����߷�ʽ
*******************************************************************/
void clear_point(u16 mode)
{
	u16 x,past_vol,pre_vol;
	
	for(x=0;x<256;x++)
	{	
		POINT_COLOR=BLACK;	//�������
		
		LCD_DrawLine( x, 50, x, 230);
		
		POINT_COLOR=WHITE;
		LCD_DrawLine(0, 140, 256, 140);		//�м�Ҵ�����
			

		pre_vol = 215-adcx[x]/4096.0*150;
	
		//���θ���
		if(mode==1)
		{
			POINT_COLOR=YELLOW;
			if(x>0&&x<255&&x!=128)	//ȥ����һ�������һ���Լ�y���ϵ�Ļ���
				LCD_DrawLine(x,past_vol,x+1,pre_vol);
		}
		else
//			lcd_huadian(x,pre_vol,YELLOW);
			POINT_COLOR=YELLOW;
			LCD_DrawPoint(x,pre_vol);											//����	
		
			past_vol = pre_vol;
	}
	
}




/******************************************************************
��飺�����ⲿ�ж����ڰ����Ķ�ȡ
WK_UP�������Ʋ�����ʾ�ĸ��º���ͣ
KEY1�������Ͳ���Ƶ��
KEY0�������Ӳ���Ƶ��
*******************************************************************/
void EXTI0_IRQHandler(void)
{
	delay_ms(10);//����
	if(WK_UP==1)	 	 //WK_UP����
	{	
		BEEP=1;
		delay_ms(50);
		BEEP=0;
		show_flag=!show_flag;
		
		POINT_COLOR=MAGENTA;
		if(show_flag)
			LCD_ShowString(260,220,200,16,16,"ing...");
		else
			LCD_ShowString(260,220,200,16,16,"stop");
	}
	EXTI_ClearITPendingBit(EXTI_Line0); //���LINE0�ϵ��жϱ�־λ  
}
 
void EXTI3_IRQHandler(void)
{
	delay_ms(10);//����
	if(KEY1==0)	 //����KEY1
	{	
		BEEP=1;
		delay_ms(50);
		BEEP=0;
		if(choice==0)
		{
			GPIO_ResetBits(GPIOF,GPIO_Pin_2);						 //PF.2�����
			GPIO_SetBits(GPIOF,GPIO_Pin_4|GPIO_Pin_6|GPIO_Pin_8);
		}
		if(choice==1)
		{
			GPIO_ResetBits(GPIOF,GPIO_Pin_4);						 //PF.2�����
			GPIO_SetBits(GPIOF,GPIO_Pin_2|GPIO_Pin_6|GPIO_Pin_8);
		}
		if(choice==2)
		{
			GPIO_ResetBits(GPIOF,GPIO_Pin_6);						 //PF.2�����
			GPIO_SetBits(GPIOF,GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_8);
		}
		if(choice==3)
		{
			GPIO_ResetBits(GPIOF,GPIO_Pin_8);						 //PF.2�����
			GPIO_SetBits(GPIOF,GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_6);
		}
		choice++;
		if(choice==4)
			choice=0;
		delay_ms(200);
		
//		pre=pre+5;
//		if(pre>72)
//		{
//			pre=1;
//		}
//		TIM_PrescalerConfig(TIM2,pre-1,TIM_PSCReloadMode_Immediate);
	}		 
	EXTI_ClearITPendingBit(EXTI_Line3);  //���LINE3�ϵ��жϱ�־λ  
}

void EXTI4_IRQHandler(void)
{
	delay_ms(10);//����
	if(KEY0==0)	 //����KEY0
	{
		BEEP=1;
		delay_ms(50);
		BEEP=0;
//		pre=pre-5;
//		if(pre<=1)
//		{
//			pre=1;
//		}
//		TIM_PrescalerConfig(TIM2,pre-1,TIM_PSCReloadMode_Immediate);
	}		 
	EXTI_ClearITPendingBit(EXTI_Line4);  //���LINE4�ϵ��жϱ�־λ  
}



/******************************************************************
��������:GetPowerMag()
��������:�������г����ֵ
����˵��:
������ע:�Ƚ�lBufOutArray�ֽ��ʵ��(X)���鲿(Y)��Ȼ������ֵ(sqrt(X*X+Y*Y)
*******************************************************************/
void GetPowerMag(void)
{
	//char str[10];
    float X,Y,Mag,magmax,mag2,mag3,mag4,mag5;//ʵ�����鲿����Ƶ�ʷ�ֵ������ֵ
    u16 i;
	float THD,thd_fz=0,thd_fm=0,THDEND;
	
	//������cr4_fft_1024_stm32
	cr4_fft_1024_stm32(fftout, fftin, NPT);	
	
    for(i=1; i<NPT/2; i++)
    {
		X = (fftout[i] << 16) >> 16;
		Y = (fftout[i] >> 16);
		
		Mag = sqrt(X * X + Y * Y); 
		FFT_Mag[i]=Mag;//���뻺�棬�����������
		//��ȡ���Ƶ�ʷ��������ֵ
		if(Mag > magmax)
		{
			magmax = Mag;
			temp = i;
		}
    }
	F=(u16)(temp/(pre/32.000));
	mag2=FFT_Mag[64];
	mag3=FFT_Mag[96];
	mag4=FFT_Mag[128];
	mag5=FFT_Mag[160];
	
	
//	sprintf(str,"%.3f",magmax);
//	LCD_ShowString(10,10,64,16,16,(char*)str);
	LCD_ShowNum(270,50,F,5,16);	
	LCD_ShowNum(10,10,magmax ,5,16);
	LCD_ShowNum(50,10,mag2,5,16);
	LCD_ShowNum(100,10,mag3,4,16);
	LCD_ShowNum(150,10,mag4,4,16);
	LCD_ShowNum(200,10,mag5,4,16);
	


	thd_fm=magmax;
	thd_fz=mag2*mag2 + mag3*mag3 + mag4*mag4+ mag5*mag5;
	thd_fz=sqrt(thd_fz);
	THD=thd_fz/thd_fm*10000;
	TTT[th]=THD;
	th++;
	//��THD��ƽ��ֵ
	if(th==5)
	{
		THDEND=(TTT[0]+TTT[1]+TTT[2]+TTT[3]+TTT[4])/5;
//		sprintf(str,"%.3f",THDEND);
		LCD_ShowString(10,10,64,16,16,(char*)THDEND);
//		LCD_ShowNum(280,10,THDEND,4,16);
		
		th=0;
	}
	
	
}






