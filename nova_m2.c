/*******************************************************
This program was created by the
CodeWizardAVR V3.12 Advanced
Automatic Program Generator
© Copyright 1998-2014 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 
Version : 
Date    : 22.01.2016
Author  : 
Company : 
Comments: 


Chip type               : ATmega16
Program type            : Application
AVR Core Clock frequency: 8,000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*******************************************************/

#include <mega16.h>
#include <stdio.h>

#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)
#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<DOR)

#asm
   .equ __w1_port=0x18 ;PORTB
   .equ __w1_bit=1
#endasm
#include <1wire.h>
#include <ds18b20.h>
#include <delay.h>
//Глобальные переменные
char input_char;
int input_count=0;
int oldTemper=0;
int rx_bufer_count=10;
int rx_bufer_index=0;
char rx_buf[rx_bufer_count];
//Инициализация UART
void intiUart(void)
{
UCSRA=(0<<RXC) | (0<<TXC) | (0<<UDRE) | (0<<FE) | (0<<DOR) | (0<<UPE) | (0<<U2X) | (0<<MPCM);
UCSRB=(1<<RXCIE) | (0<<TXCIE) | (0<<UDRIE) | (1<<RXEN) | (1<<TXEN) | (0<<UCSZ2) | (0<<RXB8) | (0<<TXB8);
UCSRC=(1<<URSEL) | (0<<UMSEL) | (0<<UPM1) | (0<<UPM0) | (0<<USBS) | (1<<UCSZ1) | (1<<UCSZ0) | (0<<UCPOL);
UBRRH=0x00;
UBRRL=0x33;
}
//Инициализация портов
void init_led(void)
{
PORTB.0=0;
DDRB.0=1;
}

//Инициализация датчика температуры
void initTemter(void)
{
w1_init();
}
void returnText()
{
int i;
UDR='_';
for( i=0; i<=rx_bufer_index;i++)
{
UDR=rx_buf[i];
delay_ms(10);
}
rx_bufer_index=0;
}



void putTemper(int temper)
{
char temp_buf[10];
int i;
               
    sprintf(temp_buf,"t=%i.%u\xdfC",temper, temper%1);             
    for(i=0; i<sizeof(temp_buf);i++)
    {
         UDR=temp_buf[i];
         delay_ms(10);
    }
//    UDR='\n\r';
}

int getTemper(void)
{
int temper;
temper=ds18b20_temperature(0);
if(temper>1000)
    {
        temper=4096-temper;
        temper=-temper;
    } 
 return temper;
}
void uartAnalis()
{
if(rx_buf[0]=='2')
{ 
 putTemper(getTemper());
}
}

//нагрузка 1
void led_on_off(void)
{
    PORTB.0=~PORTB.0;
}
//Прерывание UART
interrupt [USART_RXC] void usart_rx_isr(void)
{
char status,data;
status=UCSRA;
data=UDR;
if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
   {
       if((data==0x00)&&(rx_bufer_index!=0)) uartAnalis();
       if(data!=0x00)
       {
        rx_buf[rx_bufer_index]=data;
        rx_bufer_index++;
         
       //rx_bufer_count
        if(rx_bufer_index==rx_bufer_count)  returnText();
       }
//       input_char=data;
//       input_count=1;    
   }
  }

void main(void)
{
intiUart();
initTemter();
init_led();
#asm("sei");
putsf("Hello, world!"); 
while (1)
      {              
          if((input_char=='1')&&(input_count==1))
          {
               led_on_off();
               input_count=0;         
          }            
           if((input_char=='0')&&(input_count==1))
          {
            led_on_off();
            input_count=0;                
            putTemper(getTemper());            
          }
          if(oldTemper!=getTemper())
          {
            oldTemper=getTemper();
            putTemper(oldTemper);                
          }
      }
}

