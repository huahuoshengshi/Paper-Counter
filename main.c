#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "key.h"  
#include "24cxx.h" 
#include "myiic.h"
#include "timer.h"

extern u8  TIM2CH1_CAPTURE_STA;		//输入捕获状态		    				
extern u16	TIM2CH1_CAPTURE_VAL;	//输入捕获值

//要写入到24c02的字符串数组
	
 int main(void)
 { 
	 
	 u16 k=2;
	 u16 TEXT_Buffer[6];
	 u16 data=0;
	 u16 fq1;
	 u16 fq2;
	 u16 fq3;
	 u16 num;
	 u32 i=0;
	 u32 fq=0;                 //是否将测量的周期换算为频率
	 u32 temp=0; 
	 u8 key;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	delay_init();	    	 //延时函数初始化	  
	uart_init(9600);	 	//串口初始化为9600
	LED_Init();		  		//初始化与LED连接的硬件接口
 	LCD_Init();	
	KEY_Init();				//按键初始化		 	
	AT24CXX_Init();
	TIM2_Cap_Init(0XFFFF,72-1);		//以1MHZ的频率计数 
  //TIM1_PWM_Init(899,0);	 //PWM做实验验证波形，可删，不分频，PWM频率=72000/(899+1)=80Khz
	 
	 
	 /*LCD显示提示信息*/
	 POINT_COLOR=RED;//设置字体为红色 
	LCD_ShowString(60,50,200,16,16,"Mini STM32");	
	LCD_ShowString(60,70,200,16,16,"PaperNumber TEST");	
	LCD_ShowString(60,90,200,16,16,"Clearlove");
	LCD_ShowString(60,110,200,16,16,"2020/8/25");	
	LCD_ShowString(60,130,200,16,16,"KEY1:Write  KEY0:Read");	//显示提示信息
   
	 
	 /*检测24c02*/
	 while(AT24CXX_Check())
	{
		LCD_ShowString(60,150,200,16,16,"24C02 Check Failed!");
		delay_ms(500);
		LCD_ShowString(60,150,200,16,16,"Please Check!      ");
		delay_ms(500);
		LED0=!LED0;//DS0闪烁，PA8
	}
  LCD_ShowString(60,150,200,16,16,"24C02 Ready!");  
 
	
	/*捕获同一个频率的5次脉冲信号*/	
	while(1)
		{
	key=KEY_Scan(0);                     //不支持连续按
	if(key==KEY1_PRES)                  //KEY1按下开始写入数据
	{
		LCD_Fill(0,170,239,319,WHITE);//清除半屏    
 			LCD_ShowString(60,170,200,16,16,"Start Write 24C02");
		i=0;
	while(i<=4)
	{	 
		if(TIM2CH1_CAPTURE_STA&0X80)//成功捕获到了一次高电平,bit7 is 1 or 0
		{
			temp=TIM2CH1_CAPTURE_STA&0X3F;
			temp*=65536;					           //溢出时间总和
			temp+=TIM2CH1_CAPTURE_VAL;			//得到总的时间
			fq=1000000/temp;			          //得到频率
			TEXT_Buffer[i]=fq;              //将5次频率值储存在数组中
			//TEXT_Buffer[i]=temp;
 			TIM2CH1_CAPTURE_STA=0;			//开启下一次捕获
 		  i++;
			delay_ms(600);
		}
		
		/*未检测到脉冲，蜂鸣器报警，此处暂为LED灯闪烁*/
		else if((TIM2CH1_CAPTURE_STA&0X80)==0){
			GPIO_SetBits(GPIOD,GPIO_Pin_2);
		    delay_ms(3000);
			GPIO_ResetBits(GPIOD,GPIO_Pin_2);
		    delay_ms(3000);
      LCD_ShowString(60,170,200,16,16,"No pulse was detected!!");		
			break;
		}
	}
	
		/*计算频率的加权平均值*/
	if(i==5)                       //已经捕获到5次高电平
	{
	TEXT_Buffer[5]=TEXT_Buffer[0]*0.1
	              +TEXT_Buffer[1]*0.1
	              +TEXT_Buffer[2]*0.2
	              +TEXT_Buffer[3]*0.3
	              +TEXT_Buffer[4]*0.3;
   AT24CXX_WriteOneByte(k,TEXT_Buffer[5]/256);//取整
		AT24CXX_WriteOneByte(k+1,TEXT_Buffer[5]%256);//取余
	LCD_ShowString(60,200,200,16,16,"OK");
	LCD_ShowString(60,220,200,16,16,"Input frequency is:");
	LCD_ShowNum(60,240,TEXT_Buffer[5],16,16);
	data=AT24CXX_ReadOneByte(k)*256+AT24CXX_ReadOneByte(k+1);
	LCD_ShowNum(60,260,data,16,16);
	LCD_ShowString(60,280,200,16,16,"the PaperNumber is:");
	LCD_ShowNum(60,300,k/2,16,16);
	k+=2;
	}
}
	/*计算纸张频率*/
	if(key==KEY0_PRES)
	{
		LCD_Fill(0,170,239,319,WHITE);//清除半屏 
		LCD_ShowString(60,170,200,16,16,"Start Read 24C02");
		i=0;
		while(i<=4)
			{
		if(TIM2CH1_CAPTURE_STA&0X80)//成功捕获到了一次高电平,bit7 is 1 or 0
		{
			temp=TIM2CH1_CAPTURE_STA&0X3F;
			temp*=65536;					           //溢出时间总和
			temp+=TIM2CH1_CAPTURE_VAL;			//得到总的高电平时间
			fq=1000000/temp;			          //得到频率
			TEXT_Buffer[i]=fq;              //将5次频率值储存在数组中
			//TEXT_Buffer[i]=temp;
 			TIM2CH1_CAPTURE_STA=0;			//开启下一次捕获
 		  i++;
			delay_ms(600);
		}
		
		/*测量纸张数为0，蜂鸣器报警,此处为LED1闪烁*/		
		else if((TIM2CH1_CAPTURE_STA&0X80)==0){
			//GPIO_SetBits(GPIOA,GPIO_Pin_8);
			GPIO_SetBits(GPIOD,GPIO_Pin_2);
		    delay_ms(3000);
			GPIO_ResetBits(GPIOD,GPIO_Pin_2);
		    delay_ms(3000);
			LCD_ShowString(60,170,200,16,16,"the number is:zore");
			break;
		}
	}
			
		/*计算频率的加权平均值*/
	if(i==5)                         //同样检测频率时捕捉到5次高电平
	{
	TEXT_Buffer[5]=TEXT_Buffer[0]*0.1
	              +TEXT_Buffer[1]*0.1
	              +TEXT_Buffer[2]*0.2
	              +TEXT_Buffer[3]*0.3
	              +TEXT_Buffer[4]*0.3;
	
			
/*测量纸张数不为0*/			
   for(num=2;num<=80;num+=2)
		{
		 fq1=AT24CXX_ReadOneByte(num-2)*256+AT24CXX_ReadOneByte(num-1);
			fq2=AT24CXX_ReadOneByte(num)*256+AT24CXX_ReadOneByte(num+1);
			 fq3=AT24CXX_ReadOneByte(num+2)*256+AT24CXX_ReadOneByte(num+3);
			/*根据阈值确定测量纸张数*/
			if((fq2 -(0.4) * (fq2 - fq1) <= TEXT_Buffer[5]) 
			&& (TEXT_Buffer[5] < fq2 + (0.6)*(fq3 - fq2)))
			{
			LCD_ShowString(60,170,200,16,16,"the PaperNumber is:");
		delay_ms(1000);
	LCD_ShowNum(60,240,num/2,16,16); 
				break;
			}
			}		
		}
	}
}
}
 

