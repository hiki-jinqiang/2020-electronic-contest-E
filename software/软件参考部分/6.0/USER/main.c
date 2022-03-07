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


	 u8 adc_flag=0;//采样结束标志
	 u8 key_flag=0;//按键扫描标志
	 u8 show_flag=1;//更新暂停标志
	 u32 adcmax;//采样最大值和最小值
	 u32 adcmin;
	 u32 currentadc ;	//实时采样数据
	 u16 T=1000;	//定时器2重载值			(1000就不行)
	 u16 pre=1;		//定时器2预分频值
	
	 u32 fre;//采样频率 kHz
	
	
	
	int long fftin [NPT];//FFT输入
	int long fftout[NPT];//FFT输出
	float FFT_Mag[NPT/2]={0};//幅频特性
	u16 magout[NPT];//模拟正弦波输出缓存区
//	u16 temp=0;//幅值最大的频率成分
	u16 F;//波形频率
	
	
	u16 temp=0;//幅值最大的频率成分
//	u16 temp2=0;//幅值最大的频率成分
//	u16 temp3=0;//幅值最大的频率成分
//	u16 temp4=0;//幅值最大的频率成分
//	u16 temp5=0;//幅值最大的频率成分
//	
	float mag1;	//谐波1的幅值
//	float mag2=0;	//谐波2的幅值
//	float mag3=0;	//谐波3的幅值
//	float mag4=0;	//谐波4的幅值
//	float mag5=0;	//谐波5的幅值
//	
	
	
    int main(void)
 {	 
//	float adcx=0;
//	u32 sum=0;
//	float temp=0;
//	u16 adcy=0;
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
	TIM2_PWM_Init(T-1,pre-1);	//最大频率72000000/1/2000=36KHz
	 
	 
	 
	 
//	POINT_COLOR=RED;//设置字体为红色 
//	LCD_ShowString(60,50,200,16,16,"Elite STM32");	
//	LCD_ShowString(60,70,200,16,16,"ADC TEST");	
//	LCD_ShowString(60,90,200,16,16,"ATOM@ALIENTEK");
//	LCD_ShowString(60,110,200,16,16,"2015/1/14");	
//	//显示提示信息
//	POINT_COLOR=BLUE;//设置字体为蓝色
//	LCD_ShowString(60,130,200,16,16,"ADC_CH0_max:");	      
//	LCD_ShowString(60,150,200,16,16,"ADC_CH0_min:");	       
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
			adcmax=adcmax*0.8;   //0.8 ≈ 3300/4096	
			adcmin=adcmin*0.8;
		
			POINT_COLOR=BLUE;
			LCD_ShowxNum(280,100,adcmax,4,16,0);//显示电压最大值
			LCD_ShowxNum(280,150,adcmin,4,16,0);//显示电压最小值
			LCD_ShowxNum(280,200,adcmax-adcmin,4,16,0);//显示电压峰峰值
		
			GetPowerMag();
		
			LED0=!LED0;
			DMA_Cmd(DMA1_Channel1,ENABLE);//使能DMA1-CH1
			delay_ms(50);
		
			if(show_flag==1)
			{
				clear_point(1);    //更新显示屏当前列，采用连线绘制
			}	
	}	



		
//		adcx=Get_Adc(); 
//		LCD_ShowxNum(156,130,adcx,4,16,0);//显示ADC的值
//		temp=(float)adcx*(3.3/4096);
//		adcx=temp;
//		LCD_ShowxNum(156,150,adcx,1,16,0);//显示电压值
//		temp-=adcx;
//		temp*=1000;
//		LCD_ShowxNum(172,150,temp,3,16,0X80);
//		LED0=!LED0;
//		delay_ms(250);	
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
//	static u16 h; 
	
//	POINT_COLOR=BLUE;
//	fre=36000/pre;//更新采样频率
//	LCD_ShowNum(270,10,fre,5,16);//更新采样率显示
	
	for(x=0;x<256;x++)
	{	
		POINT_COLOR=BLACK;	//按列清除
		if(x!=128)	//去除y轴列清除
//			lcd_huaxian(x,4,x,196);
		LCD_DrawLine( x, 50, x, 230);
		
		POINT_COLOR=WHITE;
		LCD_DrawLine(0, 140, 256, 140);		//中间挂穿的线
		
		
//		//绘制坐标
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
		pre=pre+5;
		if(pre>72)
		{
			pre=1;
		}
		TIM_PrescalerConfig(TIM2,pre-1,TIM_PSCReloadMode_Immediate);
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
		pre=pre-5;
		if(pre<=1)
		{
			pre=1;
		}
		TIM_PrescalerConfig(TIM2,pre-1,TIM_PSCReloadMode_Immediate);
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
    float X,Y,Mag,magmax,mag2,mag3,mag4,mag5;//实部，虚部，各频率幅值，最大幅值
    u16 i;
	float THD,thd_fz=0,thd_fm=0;
	
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






