#include <string.h>
//$$$$$ API para trabajar con el protocolo 802.15.4 $$$$$//

#define start_trama 0x7E // Identificador presente al inicio de toda trama recibida en modo API.
#define lenght_buffer_rx 180  // Buffer para mensaje entrante, sin procesar.
#define wait_time_message 1005000 // Ciclos de espera para la funcion wait_message().

//################## Variables ##############################################//
char trama_rx[lenght_buffer_rx]; // Almacena la trama entrante en el buffer RX
                                 // del H-UART del PIC, a partir de esta trama
                                 // trabajan las funciones de desempaquetado.
int1 trama_recibida=0; // 0 = vacio; 1= trama pendiente de procesar en trama_rx.

// Desempaquetado de ultima trama recibida //----------------------------------
int16 n_caracteres=0x0000; // Tamaño total de la trama.
int8 tipo_trama=0x00; // Identificador de tipo de trama.
int8 size_mensaje=0; // Tamaño del paquete contenido en la trama.
int8 direccion_remitente16b[2]={0x00,0x00}; // Direccion PAN del remitente.
int8 direccion_remitente64b[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; // Direccion MAC del remitente.
char mensaje[lenght_buffer_rx-5]={0x00}; // Mensaje recibido

enum tipos_trama{tramaDesconocida=200, // Identificadores de trama aceptados por el Servidor. 
                 tramaBasura=255,
                 rxPacket64bAdd=0x80,
                 rxPacket16bAdd=0x81,
                 transmitStatus=0x89};
                 
//############### Funciones #################################################//

void tx_request_16(int8 *direccion,int8 *opciones, char *mensaje); // Genera una trama tx con direccion de 16 bits para empaquetar el mensaje ingresado

void wait_message(); // Espera una trama y al detectarla la almacena en trama_rx.

int1 integridad_mensaje(char trama_rx); // return 0=trama basura; 1=trama correcta

int8 clasificador_tramas(int1 integridad); // Regresa el tipo de trama detectado

void id_remitente(int16 tipo_trama); // Identifica la direccion de la cual procede el mensaje recibido (16b o 64b segun el caso)

void get_mensaje(void); // Aisla el mensaje empaquetado en trama_rx

// Envios y recepcion de trama //----------------------------------------------

void tx_request_16(int8 *direccion, int8 *opciones, char *mensaje)
{   
   int letra=0;
   int16 size_trama = strlen(mensaje) + 0x05; //0x05 numero de bytes en una trama con mensaje vacio
   int8 len[2]; 
   len[0] = size_trama>>8;
   len[1] = size_trama;
   int8 frame_type = 0x01; // tx_request_16 => 0x01
   int8 frame_id = 0x01;
   /******* Calculo de checksum *************/
   int8 checksum;
   int suma;
   suma = frame_type + frame_id + direccion[0] + direccion[1];
   while(mensaje[letra] != '\0')
   {
      suma += mensaje[letra];
      letra++;
   }
   letra = 0;
   if(suma <= 0xFF)
   {
      checksum = 0xFF - suma;
   }
   else
   {
      suma = suma & 0x00FF; // Bitmask para obtener solo el ultimo byte
      checksum = 0xFF - suma;
   }
              
              /***** Envio de trama *****/
   putc(start_trama);
   putc(len[0]); putc(len[1]);
   putc(frame_type);
   putc(frame_id);
   putc(direccion[0]);putc(direccion[1]);
   putc(opciones);
   while(mensaje[letra] != '\0') // Envia hasta detectar el caracter nulo que representa el final del string.
   {
      putc(mensaje[letra]);
      letra++;
   }
   putc(checksum);
}

void wait_message()
{
      int32 n_c=0; // Contador de caracteres entrantes
      unsigned int32 cont=0;  
      while(!kbhit(xbee)){};
      do
      {
         if(kbhit(xbee))
         {
            trama_rx[n_c++]=fgetc(xbee); // Almacena los caracteres del buffer
            cont=0;
         }
         cont++;
      }
      while(cont<wait_time_message);
      n_caracteres = n_c; // Numero total de caracteres recibidos
      
      if(n_caracteres>0){trama_recibida=1;}
      else{trama_recibida=0;}
}

// Revision y segmentacion de trama //-----------------------------------------
int1 integridad_mensaje(char *trama_rx,) 
{
      if(trama_rx[0]==start_trama) 
      {
         int16 msb_size_trama= 0x00;
         int16 lsb_size_trama = 0x00;
         int16 size_trama = 0x0000;
         msb_size_trama=trama_rx[1];
         lsb_size_trama = trama_rx[2];
         msb_size_trama= msb_size_trama<<8;
         size_trama = msb_size_trama | lsb_size_trama; // OR aplicado bit a bit para concatenar msb y lsb size_trama
         
         if(size_trama==(n_caracteres - 4)) // size_trama no considera el byte de inicio, los 2 usados para size_trama ni el checksum.
         {
            int16 suma=0x0000;
            for(int16 i=3;i<=(n_caracteres-1);i++) // Se suma toda la trama, incluyendo el checksum.
            {
               suma += trama_rx[i];
            }
            suma = suma << 8;
        
            if(suma==0xFF00)
            {
               return 1;
            }
            else
            {
               return 0;
            }
         }
         else
         {
            return 0;
         }
         
      }
      else
      {
         return 0;
      }
}

int8 clasificador_tramas(int1 integridad)
{
   // Los nombres de los tipos de trama son los utilizados por el programa XCTU
   if(integridad)
   {
      switch(trama_rx[3])
      {
      /*
         case 0x00:
            return 0; // TX 64bits request
            break;
         case 0x01:
            return 1; // TX 16bits request
            break;
         case 0x08:
            return 3; // AT Command
            break;
         case 0x09:
            return 4; // AT Command Queue Register Value 
            break;
         case 0x10:
            return 5; // Transmit Request
            break;
         case 0x11:
            return 6; // Explicit Addressing Command Frame
            break;
         case 0x17:
            return 7; // Remote AT Command
            break;
         case 0x18:
            return 8; // Secured Remote AT Command 
            break;*/
         case rxPacket64bAdd:
            return rxPacket64bAdd; // Rx(Recive) Packet 64bit Address; 0x80
            break;
         case rxPacket16bAdd:
            return rxPacket16bAdd ; // Rx(Recive) Packet 16bit Address; 0x81
            break;
         case 0x82:
            return 11; // Rx(Recive) Packet 64bit Address IO
            break;
         case 0x83:
            return 12; // Rx(Recive) Packet 16bit Address IO
            break;
         /*case 0x88:
            return 13; // AT Command Response
            break;*/
         case transmitStatus:
            return transmitStatus; // Tx(Transmit) Status; 0x89
            break;
         /*case 0x8A:
            return 15; // Modem Status
            break;
         case 0x8B:
            return 16; // Transmit Status
            break;
         case 0x90:
            return 17; // Recive Packet
            break;
         case 0x91:
            return 18; // Explicit Rx Indicator
            break;
         case 0x92:
            return 19; // IO Data Sample Rx Indicator
            break;
         case 0x97:
            return 20; // Remote AT Command Response
            break;
         case 0xAD:
            return 21; // User Data Relay Output
            break;*/ 
         default:
            return tramaDesconocida;
            break;
      }
      break; //------------------------------
   }
   else
   {
      return tramaBasura;
   }    
}

void id_remitente(void)
{
   if(tipo_trama == rxPacket16bAdd)
   {
      direccion_remitente16b[0]= trama_rx[4];
      direccion_remitente16b[1]= trama_rx[5];
   }
   
   if(tipo_trama == rxPacket64bAdd)
   {
      direccion_remitente64b[0]=trama_rx[4];
      direccion_remitente64b[1]=trama_rx[5];
      direccion_remitente64b[2]=trama_rx[6];
      direccion_remitente64b[3]=trama_rx[7];
      direccion_remitente64b[4]=trama_rx[8];
      direccion_remitente64b[5]=trama_rx[9];
      direccion_remitente64b[6]=trama_rx[10];
      direccion_remitente64b[7]=trama_rx[11];
   }
}

void get_mensaje(void)
{
   size_mensaje=0;
   if(tipo_trama==rxPacket16bAdd)
   {
      while(size_mensaje<=(n_caracteres-9)) // 8 de cabecera + 1 de cksum
      {
         mensaje[size_mensaje] = trama_rx[size_mensaje+8]; // 8 para ignorar la cabecera(bits 0-7)
         size_mensaje++;
      }
   }
   if(tipo_trama==rxPacket64bAdd)
   {
      while(size_mensaje<=(n_caracteres-15)) // 14 de cabecera + 1 de cksum
      {
         mensaje[size_mensaje] = trama_rx[size_mensaje+14]; //
         size_mensaje++;
      }
   }
}
