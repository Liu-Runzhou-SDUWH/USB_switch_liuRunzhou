#include<reg52.h>
#include<stdio.h>
#include "delay.h"
#include "i2c.h"

sbit k1=P2^4;
sbit k2=P2^5;
sbit k3=P2^6;
sbit k4=P2^7;
sbit red=P3^2;
sbit s3=P3^3;
sbit s4=P1^1;
sbit relay=P1^0;

unsigned char table[]={
0xc0, 0xf9 ,0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90,0x88};
unsigned char datas[6];        //uart datas
char *a=&datas;
char i,j= 0;
unsigned char receive,m=0;
unsigned char q=110;
unsigned char rdata,midcur;
unsigned int crt,p=0;
unsigned char dec,hun,thou=0;
bit key,flag,man,autoon,infrared=0;

unsigned long time_20ms,time,btn=0;		   //��ʱ������
float Acurrent=0.0;				  //������

void Init_Timer0(void);
void uartSendStr(unsigned char *s);
void UART_Init(void);
void uartSendByte(unsigned char dat);
void Scankey();
void LED();


void main (void)
{
	relay=1;  //off
	man=0;    //��ʱ��־����
	Init_Timer0();
	UART_Init();
	DelayMs(20);
	while(1)
	{
		q++;
		if(q>100){
			q=0;
			midcur=ReadADC(1);		//��ȡ����ת����ĵ�ѹֵ
			//DelayMs(5);
		}

		Acurrent=(float)midcur*5.13/255;		//DA
												
		if(Acurrent>2.6)
		{
			//p=0;
			Acurrent=(Acurrent-2.6)/0.184;
		}
		else
		{
			//p++;
			Acurrent=0;
		}
		crt=Acurrent*100+0.5;
		
		if(crt<4){   //too low
			p++;}      //delay
		else if(crt>450){  //too high
		relay=1;}
		else{
			p=0;
		}
		
		if(p>30000){  //delay
		relay=1;      //OFF
			key=0;
			p=0;
		}
		
		if(flag==1){     //uart��־
			TR0=1;
			m=0;
			man=0;
			if(time_20ms/50>=time){  //��ʱ��ʱ
				TR0=0;
				flag=0;
				relay=~relay;
				time=0;
				time_20ms=0;
				if(relay==0){          //���ͼ̵������
				uartSendStr("ON!\n");}
				else{uartSendStr("OFF!\n");}
			}
		}
		if(infrared==1){   //����ʹ��
		if(red==1&&man==0){  //���������ԭ������
			m++;
			if(m>200){       //���������ά��һ��ʱ��
			man=1;}}         //ȷ������
		if(red==0&&man==0){
			m=0;}
		if(red==0&&man==1&&relay==1){  //���������ԭ�������Ҽ̵�����
			m++;
			if(m>4){       //delay
			m=0;
			man=0;
			autoon=1;   //�Զ���
			relay=0;}}  //on
		if(red==1&&autoon==1){
		relay=1;   //off
		man=0;
		autoon=0;}}   //�Զ���
		
		LED();
		Scankey();
	}
}


void Init_Timer0(void)
{
	TMOD=0x01;	  //ģʽ1��16λ��ʱ��
	TH0=0xb8;		  //20ms
	TL0=0x00;
	EA=1;
	ET0=1;
}

void Timer0_isr(void) interrupt 1 
{
	TH0=0xb8;
	TL0=0x00;
	
	time_20ms++;
}


void UART_Init(void)
{
    SCON=0x50;
    TMOD|=0x20;
    TH1=0xFD;         //9600 ������
		TL1=TH1;  
    TR1=1;           //timer 1                        
    EA=1;
    ES=1;
}

void uartSendByte(unsigned char dat)
{
	unsigned char time_out;
	time_out=0x00;
	SBUF = dat;			  //�����ݷ���SBUF��
	while((!TI)&&(time_out<50))  //����Ƿ��ͳ�ȥ
	{time_out++;DelayUs2x(10);}	//δ���ͳ�ȥ ���ж�����ʱ
	TI = 0;						//���ti��־
}


void uartSendStr(unsigned char *s){
	while(*s!='\0')
	{
		uartSendByte(*s);  //���ֽ�����
		s++;
	 }
}

void UART_SER (void) interrupt 4
{
	if(RI)
	{
	  RI = 0;
		receive= SBUF;
		rdata=receive-'0';//����ת����
		for(j=0;j<5;j++){ //ѭ��ѹ������
		*(a+j)=*(a+j+1);}
		*(a+5)=rdata;     //���һλֱ��ѹ������
		if(receive=='s'){ //�������
			uartSendStr("Received!\n");
			time=datas[0]*10000+datas[1]*1000+datas[2]*100+datas[3]*10+datas[4];//תʮ������
			flag=1;
			for(j=0;j<6;j++){
		*(a+j)=0;}        //��������
			}
	}
	
	if(TI)
	{
	   TI = 0;
		}
}
void Scankey(){
	if(s4==0){
		DelayMs(5);
		if(s4==0){
			P0=0xff;
			key=~key;
			man=0;
			while(!s4){
				btn++;
				if(btn>20000){  //����
				btn=0;
				infrared=~infrared;
				autoon=0;
				if(infrared==0){ //���غ������
				uartSendStr("disabled!\n");}
				else{uartSendStr("enabled!\n");}
				while(!s4);
				}
			}
		}
	}
	if(s3==0){
		DelayMs(5);
		if(s3==0){
			P0=0xff;
			relay=~relay;
			man=0;
			while(!s3);
		}
	}
}
	
void LED(){
	dec=crt%10;
	hun=crt%100/10;
	thou=crt/100;
	//for(i=0;i<4;i++){
		P0=0xff;
	if(key==1){
        switch(i)
		{
			case 0: k1 =0;k2 =1;k3 =1;k4 =1;i++;P0=table[10];break;
			case 1: k1 =1;k2 =0;k3 =1;k4 =1;i++;P0=table[dec];break;
			case 2: k1 =1;k2 =1;k3 =0;k4 =1;i++;P0=table[hun];break;
			case 3: k1 =1;k2 =1;k3 =1;k4 =0;i=0;P0=table[thou]&0x7f;break;
		};
		DelayUs2x(25);
	}
	//}
}

