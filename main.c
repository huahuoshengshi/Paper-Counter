#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "key.h"  
#include "24cxx.h" 
#include "myiic.h"
#include "timer.h"

extern u8  TIM2CH1_CAPTURE_STA;		//���벶��״̬		    				
extern u16	TIM2CH1_CAPTURE_VAL;	//���벶��ֵ

//Ҫд�뵽24c02���ַ�������
	
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
	 u32 fq=0;                 //�Ƿ񽫲��������ڻ���ΪƵ��
	 u32 temp=0; 
	 u8 key;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	delay_init();	    	 //��ʱ������ʼ��	  
	uart_init(9600);	 	//���ڳ�ʼ��Ϊ9600
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
 	LCD_Init();	
	KEY_Init();				//������ʼ��		 	
	AT24CXX_Init();
	TIM2_Cap_Init(0XFFFF,72-1);		//��1MHZ��Ƶ�ʼ��� 
  //TIM1_PWM_Init(899,0);	 //PWM��ʵ����֤���Σ���ɾ������Ƶ��PWMƵ��=72000/(899+1)=80Khz
	 
	 
	 /*LCD��ʾ��ʾ��Ϣ*/
	 POINT_COLOR=RED;//��������Ϊ��ɫ 
	LCD_ShowString(60,50,200,16,16,"Mini STM32");	
	LCD_ShowString(60,70,200,16,16,"PaperNumber TEST");	
	LCD_ShowString(60,90,200,16,16,"Clearlove");
	LCD_ShowString(60,110,200,16,16,"2020/8/25");	
	LCD_ShowString(60,130,200,16,16,"KEY1:Write  KEY0:Read");	//��ʾ��ʾ��Ϣ
   
	 
	 /*���24c02*/
	 while(AT24CXX_Check())
	{
		LCD_ShowString(60,150,200,16,16,"24C02 Check Failed!");
		delay_ms(500);
		LCD_ShowString(60,150,200,16,16,"Please Check!      ");
		delay_ms(500);
		LED0=!LED0;//DS0��˸��PA8
	}
  LCD_ShowString(60,150,200,16,16,"24C02 Ready!");  
 
	
	/*����ͬһ��Ƶ�ʵ�5�������ź�*/	
	while(1)
		{
	key=KEY_Scan(0);                     //��֧��������
	if(key==KEY1_PRES)                  //KEY1���¿�ʼд������
	{
		LCD_Fill(0,170,239,319,WHITE);//�������    
 			LCD_ShowString(60,170,200,16,16,"Start Write 24C02");
		i=0;
	while(i<=4)
	{	 
		if(TIM2CH1_CAPTURE_STA&0X80)//�ɹ�������һ�θߵ�ƽ,bit7 is 1 or 0
		{
			temp=TIM2CH1_CAPTURE_STA&0X3F;
			temp*=65536;					           //���ʱ���ܺ�
			temp+=TIM2CH1_CAPTURE_VAL;			//�õ��ܵ�ʱ��
			fq=1000000/temp;			          //�õ�Ƶ��
			TEXT_Buffer[i]=fq;              //��5��Ƶ��ֵ������������
			//TEXT_Buffer[i]=temp;
 			TIM2CH1_CAPTURE_STA=0;			//������һ�β���
 		  i++;
			delay_ms(600);
		}
		
		/*δ��⵽���壬�������������˴���ΪLED����˸*/
		else if((TIM2CH1_CAPTURE_STA&0X80)==0){
			GPIO_SetBits(GPIOD,GPIO_Pin_2);
		    delay_ms(3000);
			GPIO_ResetBits(GPIOD,GPIO_Pin_2);
		    delay_ms(3000);
      LCD_ShowString(60,170,200,16,16,"No pulse was detected!!");		
			break;
		}
	}
	
		/*����Ƶ�ʵļ�Ȩƽ��ֵ*/
	if(i==5)                       //�Ѿ�����5�θߵ�ƽ
	{
	TEXT_Buffer[5]=TEXT_Buffer[0]*0.1
	              +TEXT_Buffer[1]*0.1
	              +TEXT_Buffer[2]*0.2
	              +TEXT_Buffer[3]*0.3
	              +TEXT_Buffer[4]*0.3;
   AT24CXX_WriteOneByte(k,TEXT_Buffer[5]/256);//ȡ��
		AT24CXX_WriteOneByte(k+1,TEXT_Buffer[5]%256);//ȡ��
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
	/*����ֽ��Ƶ��*/
	if(key==KEY0_PRES)
	{
		LCD_Fill(0,170,239,319,WHITE);//������� 
		LCD_ShowString(60,170,200,16,16,"Start Read 24C02");
		i=0;
		while(i<=4)
			{
		if(TIM2CH1_CAPTURE_STA&0X80)//�ɹ�������һ�θߵ�ƽ,bit7 is 1 or 0
		{
			temp=TIM2CH1_CAPTURE_STA&0X3F;
			temp*=65536;					           //���ʱ���ܺ�
			temp+=TIM2CH1_CAPTURE_VAL;			//�õ��ܵĸߵ�ƽʱ��
			fq=1000000/temp;			          //�õ�Ƶ��
			TEXT_Buffer[i]=fq;              //��5��Ƶ��ֵ������������
			//TEXT_Buffer[i]=temp;
 			TIM2CH1_CAPTURE_STA=0;			//������һ�β���
 		  i++;
			delay_ms(600);
		}
		
		/*����ֽ����Ϊ0������������,�˴�ΪLED1��˸*/		
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
			
		/*����Ƶ�ʵļ�Ȩƽ��ֵ*/
	if(i==5)                         //ͬ�����Ƶ��ʱ��׽��5�θߵ�ƽ
	{
	TEXT_Buffer[5]=TEXT_Buffer[0]*0.1
	              +TEXT_Buffer[1]*0.1
	              +TEXT_Buffer[2]*0.2
	              +TEXT_Buffer[3]*0.3
	              +TEXT_Buffer[4]*0.3;
	
			
/*����ֽ������Ϊ0*/			
   for(num=2;num<=80;num+=2)
		{
		 fq1=AT24CXX_ReadOneByte(num-2)*256+AT24CXX_ReadOneByte(num-1);
			fq2=AT24CXX_ReadOneByte(num)*256+AT24CXX_ReadOneByte(num+1);
			 fq3=AT24CXX_ReadOneByte(num+2)*256+AT24CXX_ReadOneByte(num+3);
			/*������ֵȷ������ֽ����*/
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
 

