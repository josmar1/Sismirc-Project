#include <funciones_servidor.h>
#include <pic_esp8266.h>
//Define PIN for software usage for SD card
#define MMCSD_PIN_SELECT  PIN_A0
#define MMCSD_PIN_SCK     PIN_A1
#define MMCSD_PIN_MOSI    PIN_A2
#define MMCSD_PIN_MISO    PIN_A3
#include <string.h>
// Include MMC/SD card driver source file
#include <mmcsd_m.c>
// Include FAT library source file
#include <fat_m.c>
char frase[]="PIC18F4550 DATALOGGER"; //Prints first message when checking the SD card
int8 i; //Variable when reading the sd; returns 0 if correct, otherwise there is a error with SD
char txt[50]; //Variable for storing the incoming data and wirting to the SD
FILE myfile;

/* El PIC asignado como Servidor cuenta con varios estados posibles con 
diversas tareas  a realizar segun el estado en el que se encuentre, las tareas 
a realizar por cada estado son hechas en las funciones handler para permitir la
rapida modificacion del comportamiento del servidor ante un estado concreto.

   Las funciones handler estan creadas a partir de las funciones de servidor,
las cuales son todas las micro-tareas que el servidor es capas de realizar   */

//############ Estados y transiciones #######################################//
enum state{REGISTRO,LIBRE,LIMPIEZAREGISTRO,INSCRIPCION,DIALOGO,ALMACENAMIENTO,WIFI}; // Estados posibles del servidor
enum state estado=REGISTRO; // Estado al que se transiciona en el siguiente ciclo

enum transiciones_posibles{iniciarDialogo,registroLLeno,nadaPendiente,falloRegistro,
                           solicitudDialogo,muchasInscripciones,maxNumeroRegistros,
                           registroExitoso,sinCredenciales,mensajeDescortes,
                           almacenarSD,finSD,envioWIFI,finWIFI};

enum dialogos{permisoHablar=0x48, // H
              permisoSolicitarServicios=0x50}; // P

enum servicios{transmitirTramaSismirc=0x54}; // T

enum transiciones_posibles transicion_activada;

int1 flag_permiso_para_hablar=0;

//############## Handlers ###################################################//

void creartxt(void);
void abrirtxtw(void);
void abrirtxta(void);
void escribirtxt(void);
void escribirtxt2(void);
void cerrartxt(void);
void verificaSD(void);


void handler_registro(void); // Revisa si exite una tabla de sensores registrados al encender.
                             // Revisa si el dispositivo intentando hablar ya esta registrado.

void handler_libre(void); // Espera un nuevo mensaje entrante si no hay tareas pendientes.
                          // Vigila el numero total de registros hechos y si se
                          // supera el numero de prevencion activa las funciones de limpieza.

void handler_limpieza(void); // Pide confirmacion de dispositivo aun presente fisicamente en red.
                             // Elimina los registros de los sensores que ya no esten activos.

void handler_inscripcion(void); // Almacena la direccion MAC asociada a la direccion
                                // PAN del sensor a registrar.

void handler_dialogo(void); // Escucha las solicitudes de servicios de parte de los sensores de la red.

void handler_almacenar(void); //Stores the received data to a SD card

void handler_WIFI(void); //Sends the data through wifi network
// Definiciones //-------------------------------------------------------------

//Creates a .txt file
void creartxt(void)
{
  if(i != 0)
  {
  fprintf(COMPU,"Error leyendo SD \r\n");
  delay_ms(200);
  }
  else
  {
      // Create a text file 'xbee.txt'
    if(mk_file("/xbee.txt") == 0)
    {
      fprintf(COMPU,"Se creo .txt \r\n");
      delay_ms(200);
    }
    else
    {
      fprintf(COMPU,"Error creando .txt \r\n");
      delay_ms(200);
    }
  }
}
//Opens a .txt file with write permision
void abrirtxtw(void)
{
    // Open the last created file 'xbee.txt' with write permission ('w')
    if(fatopen("/xbee.txt", "w", &myfile) != 0)
    {
      fprintf(COMPU,"Error abriendo .txt \r\n");
      delay_ms(200);
    }
    else
    {
    }

}
//Opens a .txt file with append permision
void abrirtxta(void)
{
    // Open the last created file 'xbee.txt' with write permission ('w')
    if(fatopen("/xbee.txt", "a", &myfile) != 0)
    {
      fprintf(COMPU,"Error abriendo .txt \r\n");
    }
    else
    {
    
    }

}
//Writes text to .txt file
void escribirtxt(void)
{
      if(i==0)
      {
            sprintf(txt,"%s\r\n",frase); // \r\n%u for leaving am empty line, %u\r\n fastly the next line
            fatputs(txt, &myfile); 
            fprintf(COMPU,"Escribiendo SD \r\n");
            delay_ms(200);
            
      }
      else
      {
        fprintf(COMPU,"Error escribiendo SD \r\n");
        delay_ms(200);
      }

}
//Writes text to .txt file
void escribirtxt2(void)
{
      if(i==0)
      {
            sprintf(txt,"%s\r\n",mensaje); // \r\n%u for leaving am empty line, %u\r\n fastly the next line
            fatputs(txt, &myfile); 
            fprintf(COMPU,"Escribiendo SD \r\n");
            delay_ms(200);
            
      }
      else
      {
        fprintf(COMPU,"Error escribiendo SD \r\n");
        delay_ms(200);
      }

}
//Closes the file for avoiding errors
void cerrartxt(void)
{
// Now close the file    
      if(fatclose(&myfile) == 0)
      {
        fprintf(COMPU,"Se guardo .txt \r\n");
        delay_ms(200);
      }
      else
      {
        fprintf(COMPU,"Error guardando .txt \r\n");
        delay_ms(200);
      }

}
//Checks if everything is Ok in SD card when starting the system
void verificaSD(void)
{

   if(i==0)
   {
      fprintf(COMPU,"Hay una SD \r\n");
      delay_ms(1000);
      if(fatopen("/xbee.txt", "a", &myfile)==0)
      {
         fprintf(COMPU,"Hay un .txt \r\n");
         cerrartxt();
         delay_ms(1000);
      }
      else
      {
         creartxt();
         abrirtxtw();
         escribirtxt();
         cerrartxt();
      }
   
   }
   else
   {
      fprintf(COMPU,"Inserte una SD \r\n");
      delay_ms(1000);
   }


}

void handler_registro(void)
{
   if(flag_permiso_para_hablar==0)
   {
      fprintf(COMPU,"# Buscando registro de dispositivos... \r\n");
      crear_registro();
      actualizar_indice();
      tipo_trama=0x00;
      transicion_activada = nadaPendiente;
   }
   if(flag_permiso_para_hablar==1)
   {
      fprintf(COMPU,"# Revisando credenciales de dispositivo... \r\n");
      if(revision_id())
      {
         fprintf(COMPU,"# PAN ID valido, iniciando comunicacion...\r\n");
         transicion_activada=iniciarDialogo;
      }
      else
      {
         fprintf(COMPU,"# Dispositivo no registrado. \r\n");
         transicion_activada=sinCredenciales;
      }
      flag_permiso_para_hablar=0;
   }
}

void handler_libre(void)
{

   esperar_mensaje();
   if(mensaje_recibido)
   {
      if(mensaje[0]==permisoHablar)
      {
         fprintf(COMPU,"# Se solicito permiso para hablar \r\n");
         flag_permiso_para_hablar=1;
         transicion_activada=solicitudDialogo;
      }
      else
      {
         fprintf(COMPU,"# Mensaje descortes, cortando comunicacion... \r\n");
         transicion_activada=mensajeDescortes;
      }
   }
   else
   {
      fprintf(COMPU,"# Sin mensajes \r\n");
      transicion_activada=nadaPendiente;
   }
}

void handler_inscripcion(void)
{
   if(formularios_usados<max_ID_disponibles)
   {
      fprintf(COMPU,"# Espacio disponible para nuevo registro \r\n");
      if(peticion_credenciales())
      {
         int8 direccion_registro_nuevo = read_eeprom(ultimo_formulario_disponible);
         fprintf(COMPU,"# Se archivara en eeprom: %d \r\n",direccion_registro_nuevo);
         archivar_formulario(direccion_registro_nuevo,&sensor);
         fprintf(COMPU,"# Sensor registrado exitosamente \r\n");
         actualizar_indice();
         transicion_activada=registroExitoso;
      }
      else
      {
         fprintf(COMPU,"# Fallo el registro, abortando comunicacion... \r\n");
         transicion_activada=falloRegistro;
      }
      
   }
   else
   {
      fprintf(COMPU,"# Registro lleno, dispositivo rechazado \r\n");
      transicion_activada=maxNumeroRegistros;
   }
   
}

void handler_dialogo(void)
{
      char chat=permisoSolicitarServicios;
      fprintf(COMPU,"# Sensor puede solicitar servicio \r\n");
      tx_request_16(direccion_remitente16b,0,chat);
      if(sensor_escucho())
      {
         esperar_mensaje();
         if(mensaje_recibido)
         {
            char servicio = mensaje[0];
            switch(servicio)
            {
               case transmitirTramaSismirc:
                  fprintf(COMPU,"# Recibiendo mediciones Sismirc \r\n");
                  //chat="OK \r\n";
                  
                  char chat1[]="Receive";
                  tx_request_16(direccion_remitente16b,0,chat1);
                  if(sensor_escucho())
                  {
                     esperar_mensaje();
                     if(mensaje_recibido)
                     {
                        fprintf(COMPU,"# Medicion Sismirc pendiente de guardado en SD:\r\n");
                        fprintf(COMPU,mensaje);
                        fprintf(COMPU," \r\n");
                        tipo_trama=0x00; //--------------------------------------------------------------------------------------------
                     }
                  }
                  else
                  {
                     fprintf(COMPU,"# Sensor corto comunicacion \r\n");
                     transicion_activada=nadaPendiente;
                  }
                  transicion_activada=almacenarSD;
                  break;
               default:
                  fprintf(COMPU,"# No se entendio la peticion, abandonando dialogo...\r\n");
                  transicion_activada=nadaPendiente;
                  break;
            }
            break;
         }
         else
         {
            fprintf(COMPU,"# Sensor dejo de reaccionar, abortando comunicacion...\r\n");
            
         }
      }
}

void handler_limpieza(void)
{

}

void handler_almacenar(void)
{
   abrirtxta();
   escribirtxt2();
   cerrartxt();
   transicion_activada=envioWIFI;
}
void handler_WIFI(void)
{
ESP8266TCPThingspeak();
ESP8266PreparaEnvio();
ESP8266EnviarThingspeak(temperatura);
fprintf(COMPU,"# Se envio a WIFI \r\n");

transicion_activada=finWIFI;

}
