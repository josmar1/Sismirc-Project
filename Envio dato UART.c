#include <18F4550.h>
#DEVICE ADC=10
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,NOPBADEN
#use delay(clock=48000000)
#include <usb/usb_bootloader.h>
#include <JACSS_CDC.c>
#USE RS232(baud=9600,rcv=pin_c7,xmit=pin_c6,bits=8,parity=N)

#DEFINE LCD_RS_PIN PIN_D0
#DEFINE LCD_RW_PIN PIN_D1
#DEFINE LCD_ENABLE_PIN PIN_D2
#DEFINE LCD_DATA4 PIN_D4
#DEFINE LCD_DATA5 PIN_D5
#DEFINE LCD_DATA6 PIN_D6
#DEFINE LCD_DATA7 PIN_D7
#INCLUDE <LCD.c>

float valoradc;
float milivolt;
int16 temperatura=0x00;

int16 H[10]={0x7E,0x00,0x06,0x01,0x02,0x00,0x00,0x00,0x48,0xB4};
int16 M[15]={0x7E,0x00,0x0B,0x01,0x02,0x00,0x00,0x00,0x37,0x33,0x36,0x38,0x38,0x43,0xA9};
int16 T[10]={0x7E,0x00,0x06,0x01,0x02,0x00,0x00,0x00,0x54,0xA8};
int16 TS[17]={0x7E,0x00,0x0D,0x01,0x02,0x00,0x00,0x00,0x53,0x65,0x6E,0x73,0x6F,0x72,0x20,0x32,0x30};
int16 S[12]={0x7E,0x00,0x08,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
int16 b=0x00;
int16 checksum=0x00;
void main()
{
   
   lcd_init();
   lcd_putc("\f");
   lcd_gotoxy(1,1);
   printf(lcd_putc,"Cliente...");   
   set_tris_b(0b00001111);
   setup_adc(ADC_CLOCK_INTERNAL);
   setup_adc_ports(AN0);
   
   while(true)
   {
   //TIEMPOS:
   //Inicio 25s
   //Hablar 12
   //Transmitir 12
   //Envio 15
   
   set_adc_channel(0);
   delay_us(10);
   valoradc=read_adc();
   milivolt=(valoradc*5000.0)/1023.0;
   //formula vo=(10mV/C)(T°C)+600
   temperatura=(milivolt-550.0)/10.0;
   S[8]=temperatura;
   b=S[3]+S[4]+S[5]+S[6]+S[7]+S[8]+S[9]+S[10];
   checksum=0xFF-b;
   S[11]=checksum;
   lcd_putc("\f");
   lcd_gotoxy(1,1);
   printf(lcd_putc,"Temp: %X",S[11]);
   output_low(pin_a4);
   delay_ms(10);
   output_high(pin_a4);
   delay_ms(500);
   
   if(input(pin_b0)==1)
   {
      int8 n=0;
      for(n=0; n<10; n++)
      {
      putc(H[n]);
      }
      lcd_putc("\f");
      lcd_gotoxy(1,1);
      printf(lcd_putc,"Hablar");
      delay_ms(1000);
      lcd_putc("\f");
   }
   
   if(input(pin_b1)==1)
   {
      int8 a=0;
      for(a=0; a<15; a++)
      {
      putc(M[a]);
      }
      lcd_putc("\f");
      lcd_gotoxy(1,1);
      printf(lcd_putc,"MAC");
      delay_ms(1000);
      lcd_putc("\f");
   }
   
   if(input(pin_b2)==1)
   {
      int8 b=0;
      for(b=0; b<10; b++)
      {
      putc(T[b]);
      }
      lcd_putc("\f");
      lcd_gotoxy(1,1);
      printf(lcd_putc,"Transmitir");
      delay_ms(1000);
      lcd_putc("\f");
   }
   
   if(input(pin_b3)==1)
   {
      int8 c=0;
      for(c=0; c<12; c++)
      {
      putc(S[c]);
      }
      lcd_putc("\f");
      lcd_gotoxy(1,1);
      printf(lcd_putc,"Trama");
      delay_ms(1000);
      lcd_putc("\f");
   }
   
   
   }
   
   
}   
