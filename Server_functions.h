#include <xbee_api.h>

//################# Database ##################################################
#define max_ID_disponibles 20 // Numero de formularios admitidos por tabla.
// Direcciones EEPROM //-------------------------------------------------------     
#define tabla_PANdb_creada 500 // Contenido de la direccion:  
                               // 0x55 = Tabla existente 0xFF = EEPROM limpia

#define ultimo_formulario_disponible 501 // Posicion de cabecera para un registro nuevo

#define inicio_formularios 0 // Direccion para iniciar la la tabla de registros


//######### Variables #########################################################
static int8 id_coordinador = 0x00;
static int8 MAC_PROPIA[3] = {0x88,0xC6,0x43};

static int8 ascii[16]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46}; // Valores ASCII 0-9 A-F

int8 indice[max_ID_disponibles];
int8 formularios_usados=0x00;
int1 mensaje_recibido=0;
struct formularioPAN // Datos para el registro de un dispositivo activo en la red
{
  int8 id;
  int8 mac_msb;
  int8 mac_mid;
  int8 mac_lsb;
};

char MAC[3] = {0x00,0x00,0x00};
                  
struct formularioPAN sensor;

//######## Funciones ##########################################################
void crear_registro(void); // Localiza el registro previo o crea uno nuevo.
void archivar_formulario(struct formularioPAN *formulario); // Guarda el ID y MAC de 1 sensor en la EEPROM.
void actualizar_indice(void); // Actualiza los ID en ram leyendo los registros en la EEPROM.
int1 revision_id(void); // Checar si el ID esta registrado.
int1 peticion_credenciales(void); // Ultimos 3 digitos de la MAC.
int1 sensor_escucho(void); // Verifica la recepcion de la ultima transmision.
int8 ascii_to_hex(char *caracter);
void esperar_mensaje(void);

void actualizar_indice(void)
{
   fprintf(COMPU,"# Actualizando indice de sensores... \r\n");
   int8 pos_indice=0;
   int8 inicio_archivo = read_eeprom(inicio_formularios);
   int8 final_archivo = read_eeprom(ultimo_formulario_disponible)-4;
   for(inicio_archivo;inicio_archivo<=final_archivo;inicio_archivo=inicio_archivo+4)
   {
      indice[pos_indice] = read_eeprom(inicio_archivo);
      fprintf(COMPU,"-->indice[%d] = %X \r\n",pos_indice,indice[pos_indice]);
      pos_indice++;
   }
   fprintf(COMPU,"# Indice con %d sensores registrados \r\n",pos_indice);
   formularios_usados=pos_indice;
}

void archivar_formulario(int8 direccion, struct formularioPAN *sensor)
{
      fprintf(COMPU,"# Archivando informacion de sensor nuevo... \r\n");
      disable_interrupts(GLOBAL); 
      //int8 inicio_formulario = read_eeprom(ultimo_formulario_disponible);
      int8 inicio_formulario = direccion;
      fprintf(COMPU,"*** inicio formulario: %X \r\n",inicio_formulario);
      write_eeprom(inicio_formulario++, sensor->id);
      fprintf(COMPU,"*** sensor ID: %X \r\n",sensor->id);
      write_eeprom(inicio_formulario++, sensor->mac_msb);
      fprintf(COMPU,"*** MAC_msb:: %X \r\n",sensor->mac_msb);
      write_eeprom(inicio_formulario++, sensor->mac_mid);
      fprintf(COMPU,"*** MAC_mid: %X \r\n",sensor->mac_mid);
      write_eeprom(inicio_formulario++, sensor->mac_lsb);
      fprintf(COMPU,"*** MAC_lsb: %X \r\n",sensor->mac_lsb);
 
      write_eeprom(ultimo_formulario_disponible, inicio_formulario);
      fprintf(COMPU,"*** siguiente formulario disponible: %X\r\n",inicio_formulario);
      enable_interrupts(GLOBAL);
      formularios_usados++;
}

void crear_registro(void)
{
    
   int8 tabla_previa_inicio = read_eeprom(tabla_PANdb_creada);
   if(tabla_previa_inicio==0xFF) // Si no hay registros, crea el primero usando los datos del servidor
   {
      fprintf(COMPU,"# No se encontro tabla de registros, creando una... \r\n");
      sensor.id = id_coordinador;
      sensor.mac_msb=MAC_PROPIA[0];
      sensor.mac_mid=MAC_PROPIA[1];
      sensor.mac_lsb=MAC_PROPIA[2];
      archivar_formulario(inicio_formularios,&sensor);
      fprintf(COMPU,"# Coordinador registrado usando ID: %d \r\n",id_coordinador);
      disable_interrupts(GLOBAL);
      write_eeprom(tabla_PANdb_creada, 0x55);
      enable_interrupts(GLOBAL);
      fprintf(COMPU,"# Se creo una tabla con capacidad para %d registros \r\n",max_ID_disponibles);
   }
   if(tabla_previa_inicio==0x55)
   {
      fprintf(COMPU,"# Se encontro una tabla de registros creada previo al arranque \r\n");
      actualizar_indice();
   }  
}

int1 revision_id(void)
{
   int8 encontrado=0;
   fprintf(COMPU,"# Iniciando busqueda del ID en el indice \r\n");
   for(int i=0; i<formularios_usados; i++)
   {
      fprintf(COMPU,"# Valor en indice[%d] : %X\r\n", i,indice[i]);
      if(indice[i] == direccion_remitente16b[1])
      {
         fprintf(COMPU,"# Dispositivo en lista \r\n");
         encontrado=1;
      }
   }
   if(encontrado){return 1;} else{return 0;}
}

void esperar_mensaje(void)
{
   fprintf(COMPU,"# Escuchando... \r\n");
   wait_message();
   if(trama_recibida)
   {
      tipo_trama = clasificador_tramas(integridad_mensaje(trama_rx));
      fprintf(COMPU,"# Trama recibida tipo: %X \r\n",tipo_trama);
      switch(tipo_trama)
      {
         case rxPacket16bAdd:
            fprintf(COMPU, "# Buscando remitente en la trama...\r\n");
            id_remitente();
            get_mensaje();
            fprintf(COMPU,"--> id_remitente: %X%X \r\n",direccion_remitente16b[0],direccion_remitente16b[1]);
            mensaje_recibido=1;
            trama_recibida=0; 
            break;
         default:
            fprintf(COMPU,"!!!!! trama de tipo desconocido !!!!!\r\n"); 
            mensaje_recibido=0;
            trama_recibida=0;
            break;
      }
      break; //----------------------
   }
   else
      {mensaje_recibido=0;}
}

int1 sensor_escucho(void)
{
   
   wait_message(); // Espera confirmacion de recibido
   if(trama_recibida)
   {
      tipo_trama = clasificador_tramas(integridad_mensaje(trama_rx));
      fprintf(COMPU,"La trama es: %X \r\n",tipo_trama);
      if(tipo_trama == transmitStatus)
      {
        if(mensaje[5] == 0x00) // Transmit Status(ok)
        {
            fprintf(COMPU," ok \r\n");
            tipo_trama=0x00; //---------------------------------------------------------------------------------------------
            return 1;
        }       
      }
      else{fprintf(COMPU," fail1 \r\n");return 0;}
   }
   else{fprintf(COMPU," fail2 \r\n");return 0;}
}

int8 ascii_to_hex(char *caracter)
{
   int8 n=0;
   int1 encontrado=0;
   while(encontrado==0 && n<=15)
   {
      //fprintf(pc," %d\r\n",n);
      if(caracter==ascii[n])
      {encontrado=1;}
      else{n++;encontrado=0;}
   }
   if(encontrado){return n;}
   else{return 255;}
}

int1 peticion_credenciales(void)
{
   fprintf(COMPU,"# Solicitando credenciales de registro... \r\n");
   char chat[] = "get ID";
   fprintf(COMPU," (IDPlease)Coordinador----->Sensor...\r\n");
   tx_request_16(direccion_remitente16b,0,chat);
   if(sensor_escucho())
   {
      esperar_mensaje();
      if(mensaje_recibido)
      {
            // Si se llega a este punto, se crea un formularioPAN con los datos enviados por el sensor
            
            fprintf(COMPU," Coordinador <----- Sensor(MAC) \r\n"); // Ultimos 3 digitos de la MAC
            fprintf(COMPU," ---Direccion: %c%c-%c%c-%c%c\r\n", mensaje[0],mensaje[1],mensaje[2],mensaje[3],mensaje[4],mensaje[5]);         
            int8 dig_ascii[2] = {0x00,0x00};
            
            sensor.id = direccion_remitente16b[1];
              
            dig_ascii[0] = ascii_to_hex(mensaje[0]) * 16;
            dig_ascii[1] = ascii_to_hex(mensaje[1]);
            sensor.mac_msb = dig_ascii[0] + dig_ascii[1];
            
            dig_ascii[0] = ascii_to_hex(mensaje[2]) * 16;
            dig_ascii[1] = ascii_to_hex(mensaje[3]);
            sensor.mac_mid = dig_ascii[0] + dig_ascii[1];
            
            dig_ascii[0] = ascii_to_hex(mensaje[4]) * 16;
            dig_ascii[1] = ascii_to_hex(mensaje[5]);
            sensor.mac_lsb = dig_ascii[0] + dig_ascii[1];
 
            return 1;
      }
      
   }
   else
   {
      fprintf(COMPU,"# Sensor no reacciona, abortando dialogo... \r\n");
      return 0;
   }
}
