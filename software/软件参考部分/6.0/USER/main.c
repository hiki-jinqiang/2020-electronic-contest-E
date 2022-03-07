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


	 u8 adc_flag=0;//����������־
	 u8 key_flag=0;//����ɨ���־
	 u8 show_flag=1;//������ͣ��־
	 u32 adcmax;//�������ֵ����Сֵ
	 u32 adcmin;
	 u32 currentadc ;	//ʵʱ��������
	 u16 T=1000;	//��ʱ��2����ֵ			(1000�Ͳ���)
	 u16 pre=1;		//��ʱ��2Ԥ��Ƶֵ
	
	 u32 fre;//����Ƶ�� kHz
	
	
	
	int long fftin [NPT];//FFT����
	int long fftout[NPT];//FFT���
	float FFT_Mag[NPT/2]={0};//��Ƶ����
	u16 magout[NPT];//ģ�����Ҳ����������
//	u16 temp=0;//��ֵ����Ƶ�ʳɷ�
	u16 F;//����Ƶ��
	
	
	u16 temp=0;//��ֵ����Ƶ�ʳɷ�
//	u16 temp2=0;//��ֵ����Ƶ�ʳɷ�
//	u16 temp3=0;//��ֵ����Ƶ�ʳɷ�
//	u16 temp4=0;//��ֵ����Ƶ�ʳɷ�
//	u16 temp5=0;//��ֵ����Ƶ�ʳɷ�
//	
	float mag1;	//г��1�ķ�ֵ
//	float mag2=0;	//г��2�ķ�ֵ
//	float mag3=0;	//г��3�ķ�ֵ
//	float mag4=0;	//г��4�ķ�ֵ
//	float mag5=0;	//г��5�ķ�ֵ
//	
	
	
    int main(void)
 {	 
//	float adcx=0;
//	u32 sum=0;
//	float temp=0;
//	u16 adcy=0;
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
	TIM2_PWM_Init(T-1,pre-1);	//���Ƶ��72000000/1/2000=36KHz
	 
	 
	 
	 
//	POINT_COLOR=RED;//��������Ϊ��ɫ 
//	LCD_ShowString(60,50,200,16,16,"Elite STM32");	
//	LCD_ShowString(60,70,200,16,16,"ADC TEST");	
//	LCD_ShowString(60,90,200,16,16,"ATOM@ALIENTEK");
//	LCD_ShowString(60,110,200,16,16,"2015/1/14");	
//	//��ʾ��ʾ��Ϣ
//	POINT_COLOR=BLUE;//��������Ϊ��ɫ
//	LCD_ShowString(60,130,200,16,16,"ADC_CH0_max:");	      
//	LCD_ShowString(60,150,200,16,16,"ADC_CH0_min:");	       
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
			
			
			if(adcx[i] >= adcmax)
			{			
				adcmax = adcx[i];
			}			
			if(adcx[i] <= adcmin)
			{			
				adcmin = adcx[i];
			}
//			temp=(float) adcx[i]*(3.3/4096);
//			printf("%3f\n",temp);
		}
			adcmax=adcmax*0.8;   //0.8 �� 3300/4096	
			adcmin=adcmin*0.8;
		
			POINT_COLOR=BLUE;
			LCD_ShowxNum(280,100,adcmax,4,16,0);//��ʾ��ѹ���ֵ
			LCD_ShowxNum(280,150,adcmin,4,16,0);//��ʾ��ѹ��Сֵ
			LCD_ShowxNum(280,200,adcmax-adcmin,4,16,0);//��ʾ��ѹ���ֵ
		
			GetPowerMag();
		
			LED0=!LED0;
			DMA_Cmd(DMA1_Channel1,ENABLE);//ʹ��DMA1-CH1
			delay_ms(50);
		
			if(show_flag==1)
			{
				clear_point(1);    //������ʾ����ǰ�У��������߻���
			}	
	}	



		
//		adcx=Get_Adc(); 
//		LCD_ShowxNum(156,130,adcx,4,16,0);//��ʾADC��ֵ
//		temp=(float)adcx*(3.3/4096);
//		adcx=temp;
//		LCD_ShowxNum(156,150,adcx,1,16,0);//��ʾ��ѹֵ
//		temp-=adcx;
//		temp*=1000;
//		LCD_ShowxNum(172,150,temp,3,16,0X80);
//		LED0=!LED0;
//		delay_ms(250);	
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
//	static u16 h; 
	
//	POINT_COLOR=BLUE;
//	fre=36000/pre;//���²���Ƶ��
//	LCD_ShowNum(270,10,fre,5,16);//���²�������ʾ
	
	for(x=0;x<256;x++)
	{	
		POINT_COLOR=BLACK;	//�������
		if(x!=128)	//ȥ��y�������
//			lcd_huaxian(x,4,x,196);
		LCD_DrawLine( x, 50, x, 230);
		
		POINT_COLOR=WHITE;
		LCD_DrawLine(0, 140, 256, 140);		//�м�Ҵ�����
		
		
//		//��������
//		POINT_COLOR=WHITE;
//		lcd_huaxian(0,0,0,200);
//		lcd_huadian(x,100,WHITE);
//		if(x == table[h])	
//		{
//			lcd_huaxian(x,1,x,3);
//			lcd_huaxian(x,101,x,103);
//			lcd_huaxian(x,199,x,197);
//			h++;
//			if(h>=16) h=0;
//		}	
//		if(x==128) 
//		{
//			lcd_huaxian(x,1,x,199);
//			for(i=10;i<200;i+=10)
//			{
//				lcd_huaxian(125,i,127,i);
//			}
//		}
		
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
		pre=pre+5;
		if(pre>72)
		{
			pre=1;
		}
		TIM_PrescalerConfig(TIM2,pre-1,TIM_PSCReloadMode_Immediate);
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
		pre=pre-5;
		if(pre<=1)
		{
			pre=1;
		}
		TIM_PrescalerConfig(TIM2,pre-1,TIM_PSCReloadMode_Immediate);
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
    float X,Y,Mag,magmax,mag2,mag3,mag4,mag5;//ʵ�����鲿����Ƶ�ʷ�ֵ������ֵ
    u16 i;
	float THD,thd_fz=0,thd_fm=0;
	
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
	F=(u16)(temp/(pre/36.0));
	mag2=FFT_Mag[57];
	mag3=FFT_Mag[85];
	mag4=FFT_Mag[114];
	mag5=FFT_Mag[142];
	
	thd_fm=magmax;
	thd_fz=mag2*mag2 + mag3*mag3 + mag4*mag4+ mag5*mag5;
	thd_fz=sqrt(thd_fz);
	THD=thd_fz/thd_fm*100;
	
//	if(T==1000)		F=(u32)((double)temp/NPT*1000  );	
//	if(T==100)		F=(u32)((double)temp/NPT*10010 );
//	if(T==10)		F=(u32)((double)temp/NPT*100200);
//	if(T==2)		F=(u32)((double)temp/NPT*249760);
	
	LCD_ShowNum(270,50,F,5,16);	
	LCD_ShowNum(10,10,magmax ,5,16);
	LCD_ShowNum(50,10,mag2,5,16);
	LCD_ShowNum(100,10,mag3,4,16);
	LCD_ShowNum(150,10,mag4,4,16);
	LCD_ShowNum(200,10,mag5,4,16);
	LCD_ShowNum(280,10,THD,4,16);
//		LCD_ShowNum(280,200,temp,4,16);					
//		LCD_ShowNum(280,220,(u32)(magmax*2.95),5,16);			
}






