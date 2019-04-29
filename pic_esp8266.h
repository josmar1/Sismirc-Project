/*
void connect(char* SSID, char* PASSWORD)
{
   printf("AT+CWJAP=\"%s\",\"%s\"",SSID,PASSWORD); 
   printf("\r\n");
   delay_ms(3000);
}
void mode(char modooperacion)
{
   printf("AT+CWMODE=%d",modooperacion); 
   printf("\r\n"); 
   delay_ms(2000);
}
*/
#use rs232(baud=115200,parity=N,BITS=8,FORCE_SW,RCV=PIN_D0,XMIT=PIN_D1,STREAM=ESP,ERRORS) //ESP8266 WIFI 

void ESP8266SetaModoEstacion(void);
void ESP8266Conectar(void);
void ESP8266MultiplesConexiones(void);
void SetupESP8266(void);
void ESP8266TCPThingspeak(void);
void ESP8266PreparaEnvio(void);
void ESP8266EnviarThingspeak(char valor);

float temperatura=30.0;
char AT[]="AT\r\n\0";
char modo[]="AT+CWMODE=1\r\n\0";
//char conectar[]="AT+CWJAP=\"Huawei p20\",\"dominguez\"\r\n\0"; //SSID y PASSWORD
//char conectar[]="AT+CWJAP=\"red_pedro\",\"8090100pedro\"\r\n\0"; //SSID y PASSWORD
//char conectar[]="AT+CWJAP=\"JOSMAR PC\",\"Aguilas1\"\r\n\0"; //SSID y PASSWORD
char conectar[]="AT+CWJAP=\"JOSMAR\",\"Aguilas1\"\r\n\0"; //SSID y PASSWORD
char multiple[]="AT+CIPMUX=1\r\n\0";
char TCP[]="AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n\0";
char preparaenvio[]="AT+CIPSEND=46\r\n\0";
char enviardato[]="GET /update?key=YQK2S08OLRGH7RRT&field1="; //campo 1
char enviardato2[]="GET /update?key=YQK2S08OLRGH7RRT&field2="; //campo 2
//I4S1VKUHUWGF8RI0 Hygrometer
//YQK2S08OLRGH7RRT LM61

void ESP8266SetaModoEstacion(void)
{
   puts(AT,ESP);
   delay_ms(1000);
   fprintf(COMPU,"----MODO \r\n");
   puts(modo,ESP);
   delay_ms(1000);
}

void ESP8266Conectar(void)
{
   fprintf(COMPU,"----Conectar \r\n");
   puts(conectar,ESP);
   delay_ms(5000);
}

void ESP8266MultiplesConexiones(void)
{
   fprintf(COMPU,"----MULTIPLE \r\n");
   puts(multiple,ESP);
   delay_ms(1000);
}

void SetupESP8266(void)
{
   fprintf(COMPU,"----SETUP \r\n");
   ESP8266SetaModoEstacion();
   ESP8266Conectar();
   ESP8266MultiplesConexiones();
}

void ESP8266TCPThingspeak(void)
{
   fprintf(COMPU,"----TCP \r\n");
   puts(TCP,ESP);
   delay_ms(2000);
}

void ESP8266PreparaEnvio(void)
{
   fprintf(COMPU,"----PREPARA ENVIO \r\n");
   puts(preparaenvio,ESP);
   delay_ms(1000);   
}

void ESP8266EnviarThingspeak(char valor)
{
   char EnvioHTTP[50];
   fprintf(COMPU,"----ENVIAR \r\n");
   memset(EnvioHTTP,0,sizeof(EnvioHTTP));
   sprintf(EnvioHTTP,"%s%03d\r\n\0",enviardato2,valor);
   puts(EnvioHTTP,ESP);
   delay_ms(2000);
}


