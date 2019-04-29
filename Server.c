#include<18F46K20.h>
//#include<18f4550.h>
#device PASS_STRINGS = IN_RAM
#FUSES WDT128,NOBROWNOUT,MCLR,NOLVP,NOXINST
#use delay(internal=16000000)
//#use delay(clock=8M)
#use rs232(BAUD=9600,BITS=8,UART1,STREAM=xbee,ERRORS) //Xbee module
#use rs232(BAUD=9600,BITS=8,FORCE_SW,RCV=PIN_D3, XMIT=PIN_D2,STREAM=COMPU,LONG_DATA) //USB to TTL converter
#include <servidor.h> // Cambiar a manejador de estado

void main(void)
{ 
   fprintf(COMPU,"--- Maquina de estados iniciada ---\r\n");
   delay_ms(10000); //Necesario para inicio correcto de ESP8266
   SetupESP8266(); //Configura el ESP8266 para la carga de datos a Thingspeak
   i = fat_init(); //Checa si hay tarjeta SD, devuelve 0 si todo es correcto
   verificaSD();  //Checa que se pueda escrbir en la tarjera SD
   fprintf(COMPU,"La trama es: %X \r\n",tipo_trama); 
   
   while(TRUE)
   {

      switch(estado)
      {
//-----------------------------------------------------------------------------
         case REGISTRO:
            fprintf(COMPU,">>>>> ESTADO: Registro <<<<< \r\n");
            handler_registro();
               switch(transicion_activada)
               {
                  case iniciarDialogo:
                     estado=DIALOGO;
                     break;
                  case sinCredenciales:
                     estado=INSCRIPCION;
                     break;
                  case nadaPendiente:
                     estado=LIBRE;
                     break;
               }
            break;
//-----------------------------------------------------------------------------
         case LIBRE:
            fprintf(COMPU,">>>>> ESTADO: Libre <<<<< \r\n");
            handler_libre();
               switch(transicion_activada)
               {
                  case solicitudDialogo:
                     estado=REGISTRO;
                     break;
                  case mensajeDescortes:
                     estado=LIBRE;
                     break;
                  case nadaPendiente:
                     estado=LIBRE;
                     break;
               }
            break;
//-----------------------------------------------------------------------------
         case LIMPIEZAREGISTRO:
            fprintf(COMPU,">>>>> ESTADO: Limpiando registro de sensores <<<<< \r\n");
            handler_limpieza();
               estado=LIBRE;
            break;
//-----------------------------------------------------------------------------
         case INSCRIPCION:
            fprintf(COMPU,">>>>> ESTADO: Inscripcion sensor a la red <<<<< \r\n");
            handler_inscripcion();
               switch(transicion_activada)
               {
                  case maxNumeroRegistros:
                     estado=LIMPIEZAREGISTRO;
                     break;
                  case falloRegistro:
                     estado=LIBRE;
                     break;
                  case registroExitoso:
                     estado=DIALOGO;
                     break;
               }
            break;
//-----------------------------------------------------------------------------
         case DIALOGO:
            fprintf(COMPU,">>>>> ESTADO: Dialogando con sensor <<<<< \r\n");
            handler_dialogo();
            switch(transicion_activada)
            {
                case almacenarSD:
                  estado=ALMACENAMIENTO;
                  break;
            }
            break;
//-----------------------------------------------------------------------------            
         case ALMACENAMIENTO:
            fprintf(COMPU,">>>>> ESTADO: Almacenando en SD <<<<< \r\n");
            handler_almacenar();
            switch(transicion_activada)
            {
               case envioWIFI:
                  estado=WIFI;
                  break;
            }
            break;
//-----------------------------------------------------------------------------
            case WIFI:
               fprintf(COMPU,">>>>> ESTADO: Enviando por WIFI <<<<< \r\n");
               handler_WIFI();
               switch(transicion_activada)
               {
                  case finWIFI:
                     estado=LIBRE;
                     output_low(PIN_B0);              
                     break;     
               }
               break;
      }
      
  }
  
}
