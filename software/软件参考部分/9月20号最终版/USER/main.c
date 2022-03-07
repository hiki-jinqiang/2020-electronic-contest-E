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
stm32示波器设计
PA4为模拟电压输入口
************************************************/

#define NPT  1024

void clear_point(u16 mode);
void GetPowerMag(void);

	
	int t,i;
	u32 adcx[NPT];//adc数值缓存


	 u8 adc_flag=0;	//采样结束标志
	 u8 key_flag=0;	//按键扫描标志
	 u8 show_flag=1;	//更新暂停标志
	 u32 adcmax;	//采样最大值和最小值
	 u32 adcmin;
	 u32 currentadc ;	//实时采样数据
	 u16 T=2250;		//定时器2重载值			(修改plus便是)
	 u16 pre=1;			//定时器2预分频值
	 u32 fre;			//采样频率 kHz
	 int choice=0;		//按键变量
	
	
//求FFT模式下的的变量
	int long fftin [NPT];//FFT输入
	int long fftout[NPT];//FFT输出
	float FFT_Mag[NPT/2]={0};//幅频特性
	u16 magout[NPT];//模拟正弦波输出缓存区

	float mag1;			//谐波1的幅值
	float TTT[5]={0};		//存放THD的数组
	int th=0;
	float temp;
	float F;
	
	
    int main(void)
 {	 

	MYDMA1_Config(DMA1_Channel1,(u32)&ADC1->DR,(u32)&currentadc,1);
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
 	LED_Init();			     //LED端口初始化
	EXTIX_Init();			//外部中断初始化	
	BEEP_Init();			//初始化
	LCD_Init();
	LED_Init();		//led初始化
 	Adc_Init();		  		//ADC初始化
	window();				//界面初始化
	TIM2_PWM_Init(T-1,pre-1);	//最大频率72000000/1/1000=72KHz
	 
	 
	 
	 
       
	while(1)
	{
		LED0=0;
		//等待采样完成
		
		while(adc_flag==0)
		{
			LED1=!LED1;
		}
		adc_flag=0;
		
		//获取最大最小值
		adcmax=adcx[1];
		adcmin=adcx[1];
		for(i=0;i<NPT;i++)
		{
			fftin[i] = 0;
			fftin[i] = adcx[i] << 16;
			
			//求电压的最大值和最小值
			if(adcx[i] >= adcmax)
			{			
				adcmax = adcx[i];
			}			
			if(adcx[i] <= adcmin)
			{			
				adcmin = adcx[i];
			}
		}
			adcmax=adcmax*0.799;   //0.8 ≈ 3300/4096	
			adcmin=adcmin*3300/4096;
		
			POINT_COLOR=BLUE;
			LCD_ShowxNum(280,100,adcmax,4,16,0);//显示电压最大值
			LCD_ShowxNum(280,150,adcmin,4,16,0);//显示电压最小值
			LCD_ShowxNum(280,200,adcmax-adcmin,4,16,0);//显示电压峰峰值
		
			GetPowerMag();
		
			LED0=!LED0;
			DMA_Cmd(DMA1_Channel1,ENABLE);//使能DMA1-CH1
			delay_ms(10);
		
			if(show_flag==1)
			{
				clear_point(1);    //更新显示屏当前列，采用连线绘制
			}	
	}	



		

	}
 
	
//DMA中断设置函数，将ADC采集的数据转移到数组中	
	
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
					DMA_Cmd(DMA1_Channel1, DISABLE);        //失能DMA
				}
			}
			DMA_ClearITPendingBit(DMA1_IT_TC1);		
		}
		
		
		
 /******************************************************************
函数名称:clear_point()
函数功能:循环更新波形
参数说明:mode 波形模式选择 1——连线模式，0——打点模式
备    注:波形的显示可采用打点方式和绘制线方式
*******************************************************************/
void clear_point(u16 mode)
{
	u16 x,past_vol,pre_vol;
	
	for(x=0;x<256;x++)
	{	
		POINT_COLOR=BLACK;	//按列清除
		
		LCD_DrawLine( x, 50, x, 230);
		
		POINT_COLOR=WHITE;
		LCD_DrawLine(0, 140, 256, 140);		//中间挂穿的线
			

		pre_vol = 215-adcx[x]/4096.0*150;
	
		//波形更新
		if(mode==1)
		{
			POINT_COLOR=YELLOW;
			if(x>0&&x<255&&x!=128)	//去除第一个，最后一个以及y轴上点的绘制
				LCD_DrawLine(x,past_vol,x+1,pre_vol);
		}
		else
//			lcd_huadian(x,pre_vol,YELLOW);
			POINT_COLOR=YELLOW;
			LCD_DrawPoint(x,pre_vol);											//画点	
		
			past_vol = pre_vol;
	}
	
}




/******************************************************************
简介：三个外部中断用于按键的读取
WK_UP按键控制波形显示的更新和暂停
KEY1按键降低采样频率
KEY0按键增加采样频率
*******************************************************************/
void EXTI0_IRQHandler(void)
{
	delay_ms(10);//消抖
	if(WK_UP==1)	 	 //WK_UP按键
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
	EXTI_ClearITPendingBit(EXTI_Line0); //清除LINE0上的中断标志位  
}
 
void EXTI3_IRQHandler(void)
{
	delay_ms(10);//消抖
	if(KEY1==0)	 //按键KEY1
	{	
		BEEP=1;
		delay_ms(50);
		BEEP=0;
		if(choice==0)
		{
			GPIO_ResetBits(GPIOF,GPIO_Pin_2);						 //PF.2输出低
			GPIO_SetBits(GPIOF,GPIO_Pin_4|GPIO_Pin_6|GPIO_Pin_8);
		}
		if(choice==1)
		{
			GPIO_ResetBits(GPIOF,GPIO_Pin_4);						 //PF.2输出低
			GPIO_SetBits(GPIOF,GPIO_Pin_2|GPIO_Pin_6|GPIO_Pin_8);
		}
		if(choice==2)
		{
			GPIO_ResetBits(GPIOF,GPIO_Pin_6);						 //PF.2输出低
			GPIO_SetBits(GPIOF,GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_8);
		}
		if(choice==3)
		{
			GPIO_ResetBits(GPIOF,GPIO_Pin_8);						 //PF.2输出低
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
	EXTI_ClearITPendingBit(EXTI_Line3);  //清除LINE3上的中断标志位  
}

void EXTI4_IRQHandler(void)
{
	delay_ms(10);//消抖
	if(KEY0==0)	 //按键KEY0
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
	EXTI_ClearITPendingBit(EXTI_Line4);  //清除LINE4上的中断标志位  
}



/******************************************************************
函数名称:GetPowerMag()
函数功能:计算各次谐波幅值
参数说明:
备　　注:先将lBufOutArray分解成实部(X)和虚部(Y)，然后计算幅值(sqrt(X*X+Y*Y)
*******************************************************************/
void GetPowerMag(void)
{
	//char str[10];
    float X,Y,Mag,magmax,mag2,mag3,mag4,mag5;//实部，虚部，各频率幅值，最大幅值
    u16 i;
	float THD,thd_fz=0,thd_fm=0,THDEND;
	
	//调用自cr4_fft_1024_stm32
	cr4_fft_1024_stm32(fftout, fftin, NPT);	
	
    for(i=1; i<NPT/2; i++)
    {
		X = (fftout[i] << 16) >> 16;
		Y = (fftout[i] >> 16);
		
		Mag = sqrt(X * X + Y * Y); 
		FFT_Mag[i]=Mag;//存入缓存，用于输出查验
		//获取最大频率分量及其幅值
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
	//求THD的平均值
	if(th==5)
	{
		THDEND=(TTT[0]+TTT[1]+TTT[2]+TTT[3]+TTT[4])/5;
//		sprintf(str,"%.3f",THDEND);
		LCD_ShowString(10,10,64,16,16,(char*)THDEND);
//		LCD_ShowNum(280,10,THDEND,4,16);
		
		th=0;
	}
	
	
}






