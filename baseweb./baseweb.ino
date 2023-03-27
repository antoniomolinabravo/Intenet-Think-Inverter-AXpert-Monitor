//
//  MICROCONTROLADOR ESP32 MONITOREO INVERSOR SOLAR AXPERT 3KV
//  AUTOR: ANTONIO MOLINA BRAVO
//  
//  FASE 1: COMUNICACION ESP32/INVERTER
//  FASE 2: SECUENCIA DE COMANDOS
//  FASE 3: ENVIO DATOS VIA WEB SERVER ASINCRONO
//  FASE 4: DATA LOGGER
//  FASE 5: ANALISIS DE DATOS
//  FASE 6: INCORPORAR MEDICIONES INDEPENDIENTES O EXTERNAS AL INVERTER
//  FASE 7: TOMA DE DESICION AUTONOMA Y CAMBIO DE PARAMETROS
//

// PROTOCOLO    BROWSER <-http-> MICRO
// consultas (get):     /cmd?Qxxx=1
// respuestas (json):   Qxxx:yyyyyy
// eventos (json):      Qxxx:yyyyyy
// --------------------------------------

// PROTOCOLO    MICRO <-serial-> INVERTER
// SERIAL (2400 bps, 8 datos, 1 bit paro, sin paridad)
// consultas (hex):     Qxxx CRC CR
// respuestas (txt):    yyyyyy CRC CR
// --------------------------------------

//
//  INVERTER SERIAL PORT
//
//      UTP                           RS232                 NULL MODEM MAX3232        MAX232 ESP32
//       1 WHITE ORANGE                2 TX                 3 RX                              16 RX
//       2 BLUE                        3 RX                 2 TX                              17 TX
//       8 WHITE GREEN                 5 GROUND             5 GROUND                          GND
//


// CMD  TXT CRC(2)+CR   HEX -->   <-- TXT                  <-- HEX CRC(2)+CR
// QID: 51-49-44-D6-EA-0D         <-- rs:(92332101103535   ej:28-39-32-33-33-32-31-30-31-31-30-33-35-33-35-19-39-0D
// Solicita el ID del equipo
// Retorna el numero de serie
//
// QPI: 51-50-49-BE-AC-0D         <-- rs:(PI30             ej:28-50-49-33-30-9A-0B-0D
// Solicita la version de comunicacion
// Retorna PI30
//
// QID: 51-49-44-D6-EA-0D         <-- rs:(92332101103535   ej:28-39-32-33-33-32-31-30-31-31-30-33-35-33-35-19-39-0D
// Solicita ID equipo
// Retorna 92332101103535
//
// QVFW: 51-56-46-57-62-99-0D     <-- rs:(VERFW:00006.23   ej:28-56-45-52-46-57-3A-30-30-30-30-36-2E-32-33-31-14-0D
// Solicita version de Firmware
// Retorna version
//
// QVFW2: 51-56-46-57-32-C3-F5-0D <-- rs:(VERFW2:00001.12  ej:28-56-45-52-46-57-32-3A-30-30-30-30-31-2E-31-32-D3-1C-0D
// Solicita version de Firmware2
// Retorna version
//
// QPIRI: 51-50-49-52-49-F8-54-0D <-- rs:(230.0 13.0 230.0 50.0 13.0 3000 3000 24.0 23.0 21.0 28.2 27.0 0 15 20 0 2 3 - 0 1 1 0 27.0 0 0
// Solicita configuracion del equipo
// Retorna datos parametros min y maximos de operacion
//
// QFLAG: 51-46-4C-41-47-98-74-0D <-- rs:(EayzDbjkuvx      ej:28-45-61-79-7A-44-62-6A-6B-75-76-78-6A-A3-0D
// Solicita estado de Flags
// Retorna dos listas juntas primero las E enableds  y seguido las D disableds, las letras identifican el codigo
//
// QPIGS: 51-50-49-47-53-B7-A9-0D <-- rs:(001.0 00.0 228.0 50.0 0000 0000 000 345 24.19 000 062 0418 0000 000.0 24.92 000 01 10010000 00 04 00000 000
// Solicita mediciones de operacion
// Retorna telemetria
/*
(                 START BYTE
001.0             Grid voltage (V)
 00.0             Grid frequency (Hz)
 228.0            AC output voltage (V)
 50.0             AC output frequency (Hz)
 0000             AC output apparent power (VA)
 0000             AC output active power (W)
 000              Output load percent (%)           el maximo entre VA y W   y considerando que el 100% son 3Kw
 345              BUS voltage (V)                       ????
 24.19            Battery voltage (V)
 000              Battery charging current (A)       considerar que la bateria carga a 24V
 062              Battery capacity (%)               100% ???
 0418             Inverter heat sink temperature (°C)   ????
 0000             PV Input current for battery (A)   considerar que la bateria carga a 24V pero el panel da entre 30 y 80V
 000.0            PV Input voltage 1 (V)
 24.92            Battery voltage from SCC (V)          ????
 00001            Battery discharge current (A)
 10010000         Device status (8bit)              mas detalle mas abajo DEVICE STATUS
 00               
 04
 00000
 000
*/
//
// QMOD: 51-4D-4F-44-49-C1-0D     <-- rs:(B                ej:28-42-E7-C9-0D
// Solicita estado operacion
// Retorna estado
//
// QPIWS: 51-50-49-57-53-B4-DA-0D <-- rs:(00000100000000000000000000000000
// Solicita estado activos
// Retorna estados 1 habilitado, 0 deshabilitado
//
// QMCHGCR: 51-4D-43-48-47-43-52-D8-55-0D <-- rs:(020 030 040 050 060
// Solicita rangos de operacion
// Retorna rangos
// ----------------------------------------------------------------------------------------------------------------------------------------------------



#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>    /* https://github.com/me-no-dev/ESPAsyncWebServer  */
#include "SPIFFS.h"
#include <ESPmDNS.h>

#define RXD2 16
#define TXD2 17

// API RESP    https://www.luisllamas.es/como-servir-y-consumir-un-api-rest-con-esp8266-y-axios/
// https://randomnerdtutorials.com/esp32-async-web-server-espasyncwebserver-library/

String QPIGS = "\x51\x50\x49\x47\x53\xB7\xA9\x0D";
String QID = "\x51\x49\x44\xD6\xEA\x0D";
String QPI = "\x51\x50\x49\xBE\xAC\x0D";
String QVFW = "\x51\x56\x46\x57\x62\x99\x0D";
String QVFW2 = "\x51\x56\x46\x57\x32\xC3\xF5\x0D";
String QPIRI = "\x51\x50\x49\x52\x49\xF8\x54\x0D";
String QFLAG = "\x51\x46\x4C\x41\x47\x98\x74\x0D";
String QMOD = "\x51\x4D\x4F\x44\x49\xC1\x0D";
String QPIWS = "\x51\x50\x49\x57\x53\xB4\xDA\x0D";

// Replace with your network credentials
const char* ssid = "avespana3002";
const char* password = "Arica2020";
 
const char *soft_ap_ssid = "ESP32AP";
const char *soft_ap_password = "Arica2020";

boolean Emulate = false;

const int LED_BUILTIN = 2;
int LED = LOW;

unsigned long onOperative = 0;
unsigned long onTime = 0;
String onDateJSON = "1900-01-01T04:00:00.000Z"; // hora 4 por la zona de chile

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");



void printParams(AsyncWebServerRequest *request) {
int paramsNr = request->params();
Serial.print("Param cnt: ");
Serial.println(paramsNr);

String sensorId = request->pathArg(0);
Serial.print("PathArg: ");
Serial.println(sensorId);

  for(int i=0;i<paramsNr;i++){
     AsyncWebParameter* p = request->getParam(i);
     Serial.print("Param name: ");
     Serial.println(p->name());
 
     Serial.print("Param value: ");
     Serial.println(p->value());
 
     Serial.println("------");
  }
}

void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  //WiFi.mode(WIFI_STA);
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  if(!MDNS.begin("esp32")) {
    Serial.println("Error iniciando mDNS");
  }
  
  Serial.println("Local IP:");
  Serial.println(WiFi.localIP());
  
  Serial.print("ESP32 IP as soft AP: ");
  Serial.println(WiFi.softAPIP());
}

void initSerial(){
	Serial.begin(115200);
	Serial2.begin(2400, SERIAL_8N1, RXD2, TXD2);
}

unsigned long long stringToLong(String s)
{
   char arr[12];
   s.toCharArray(arr, sizeof(arr));
   return atoll(arr);
}



void setup() {
  pinMode (LED_BUILTIN, OUTPUT);

	initSerial();
	initWiFi();
	initSPIFFS();

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //printParams(request);
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/");

  server.on("/cmdReset", HTTP_GET, [](AsyncWebServerRequest *request){
    //printParams(request);
//    gyroX=0;
//    gyroY=0;
//    gyroZ=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/cmdEmulate", HTTP_GET, [](AsyncWebServerRequest *request){
    //printParams(request);
    Emulate = !Emulate;
    Serial.println("<-EMU: " + Emulate);
    request->send(200, "text/plain", "OK");
  });
  
  server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request){
    //printParams(request);
     AsyncWebParameter* p = request->getParam(0);
     Serial.print("Param name: ");
     Serial.println(p->name());
     Serial.print("Param value: ");
     Serial.println(p->value());

    if(p->name() == "setTime") {
    //setTime=0;
      onTime = millis()/1000;               // sera su segundo zero
      onDateJSON = p->value();   // request->pathArg(0);     // parametro
      //strcpy(onDateJSON, String(p->value()));
      //sprintf(onDateJSON, "%s", p->value().c_str());
      onDateJSON = String(p->value());
      Serial.println("<-SETTIME: " + onDateJSON);
    }

//      const char * c_str() const {return _buffer;}
//    long x = strtol(str.c_str(), NULL, 10);

    if(p->name() == "setSegundos") {
      //onDateJSON = p->value();   // request->pathArg(0);     // parametro
        //strcpy(onDateJSON, String(p->value()));
        //sprintf(onDateJSON, "%s", p->value().c_str());
        char tempo[80];
        sprintf(tempo, "%s%c", String(p->value()), char(0));
      //onOperative = atol(tempo); // (onOperative<60)? 60-5: (onOperative<3600)? 3600-5: (onOperative<86400)? 86400-5: 0;  //long(String(p->value()));
      onOperative = stringToLong (tempo);
      Serial.println("<-SETSEGUNDOS: " + String(p->value()) + " <- " + onOperative);      
    }

    request->send(200, "text/plain", "OK");
  });

  server.on("/cmdResetESP", HTTP_GET, [](AsyncWebServerRequest *request){
    //printParams(request);
Serial.println("Restarting in 3 seconds");
delay(3000);
//ESP.restart();
    Serial.println("<-RST: ");
    request->send(200, "text/plain", "OK");
  });

    // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 10 second
    client->send("hello!", NULL, millis(), 10000);
  });
  
  // Servicio que enviara mensajes de actualizacion a los clientes
  server.addHandler(&events);

  server.begin();
}

void loop() {
int i=0;
int delta = 950;

  serialCmd(QPI);
    delay(delta);
    sendMsg("");
    serialCmd(QPIGS);
    delay(delta);
    sendMsg("");
  serialCmd(QID);
    delay(delta);
    sendMsg("");
    serialCmd(QPIGS);
    delay(delta);
    sendMsg("");
  serialCmd(QVFW);
    delay(delta);
    sendMsg("");
    serialCmd(QPIGS);
  delay(delta);
    sendMsg("");
  serialCmd(QVFW2);
    delay(delta);
    sendMsg("");
    serialCmd(QPIGS);
  delay(delta);
    sendMsg("");
  serialCmd(QFLAG);
    delay(delta);
    sendMsg("");
    serialCmd(QPIGS);
  delay(delta);
    sendMsg("");
  serialCmd(QMOD);
    delay(delta);
    sendMsg("");
    serialCmd(QPIGS);
  delay(delta);
    sendMsg("");
  serialCmd(QPIRI);
    delay(delta);
    sendMsg("");
    serialCmd(QPIGS);
  delay(delta);
    sendMsg("");
  serialCmd(QPIWS);

  delay(delta);
sendIP();

  while(i<=20) {
    i++;
    delay(delta);
    sendMsg("");
    serialCmd(QPIGS);
  } i=0;
  
/*  if ((millis() - lastTimeAcc) > accelerometerDelay) {
    // Send Events to the Web Server with the Sensor Readings
    //events.send(dato.c_str(),"accelerometer_readings",millis()); // getAccReadings().c_str(),"accelerometer_readings",millis());
    sprintf(temp, "Seconds since boot: %u", millis()/1000);
    events.send(temp, "temperature_reading", millis()/1000); //send event "time"
    lastTimeAcc = millis();
  }
*/
}

void serialCmd(String command) {
  String stringOne = "";
  String command2;
  Serial2.print(command);
  Serial.println("-> " + command);
  delay(30);
  stringOne = Serial2.readString();
  
  command2 = command.substring(0,command.length()-3);
  
if(Emulate) {
  if(command2 == "QPIGS") stringOne = "(001.0 00.0 228.0 50.0 0000 0010 000 345 24.19 001 062 0418 0000 000.0 24.92 002 01 10010000 00 04 00002 000ABC";
  if(command2 == "QPI") stringOne = "(PI30ABC";
  if(command2 == "QID") stringOne = "(EMU02101103535ABC";
  if(command2 == "QVFW") stringOne = "(VERFW:00006.23ABC";
  if(command2 == "QVFW2") stringOne = "(VERFW2:00001.12ABC";
  if(command2 == "QFLAG") stringOne = "(EayzDbjkuvxABC";
  if(command2 == "QPIRI") stringOne = "(230.0 13.0 230.0 50.0 13.0 3000 3000 24.0 23.0 21.0 28.2 27.0 0 15 20 0 2 3 - 0 1 1 0 27.0 0 0ABC";
  if(command2 == "QMOD") stringOne = "(BABC";
  if(command2 == "QPIWS") stringOne = "(00000100000000000000000000000000ABC";
  if(command2 == "QMCHGCR") stringOne = "(020 030 040 050 060ABC";
}

  if(command2 == "QPIGS") split(stringOne);
  stringOne = command2+stringOne.substring(0,stringOne.length()-3);

  Serial.println ("<- " + stringOne);
  events.send(stringOne.c_str(), command2.c_str(), millis()/1000); //send event "time"
}

void sendMsg(String message) {
  char temp[80] = "";
  char tmp_time[30] = "";

  //text.toCharArray( text_array,text.length() );
  //Sting(text_array);
  onDateJSON.toCharArray( tmp_time, onDateJSON.length() );
  sprintf(temp, "SEG(%s %u %u %s Z", tmp_time, (millis()/1000)-onTime, (millis()/1000)-onOperative, message);
//  temp = (millis()/1000).c_str() + message;
  Serial.println("t-> " + onDateJSON); 
  Serial.println("m-> " + String(temp)); // + temp.c_str());
  events.send(temp, "message", millis()/1000); //send event "time"

  LED = (!Emulate)? LOW : HIGH;  // (LED == HIGH)
  digitalWrite (LED_BUILTIN, LED);  // turn on/off the LED
}

void sendIP() {
  char temp[80] = "";
  //Serial.print("ESP32 IP: ");
  //Serial.println(WiFi.localIP());
  
  //Serial.print("ESP32 IP as soft AP: ");
  //Serial.println(WiFi.softAPIP());

  //sprintf(temp, "IP(%s:%s %s Z", WiFi.status(), WiFi.localIP(), WiFi.softAPIP() );
  sprintf(temp, "IP(%s %s Z", String(WiFi.localIP()), String(WiFi.softAPIP()) );
  Serial.println("m-> " + String(temp)); // + temp.c_str());
  events.send(temp, "IP", millis()/1000); //send event "IP"
}

void split(String linea) {
  // (001.0 00.0 228.0 50.0 0000 0000 000 345 24.19 000 062 0418 0000 000.0 24.92 00001 10010000 00 04 00000 000
  //  A     B    C     D    E    F    G   H   I     J   K   L    M    N     O     P     Q        R  S  T     U
  // 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
  // 0         1         2         3         4         5         6         7         8         9         10
  //
  // DATO POS   LARGO
  // A:   1     5   001.0        A 00   Grid voltage (V)
  // B:   7     4   00.0         B 01   Grid frequency (Hz)
  // C:   12    5   228.0        C 02   AC output voltage (V)
  // D:   18    4   50.0         D 03   AC output frequency (Hz)
  // E:   23    4   0000         E 04   AC output apparent power (VA)
  // F:   28    4   0000         F 05   * AC output active power (W)
  // G:   33    3   000          G 06   * Output load percent (%)           el maximo entre VA y W   y considerando que el 100% son 3Kw
  // H:   37    3   345          H 07   BUS voltage (V)                       ????
  // I:   41    5   24.19        I 08   * Battery voltage (V)
  // J:   47    3   000          J 09   * Battery charging current (A)       considerar que la bateria carga a 24V
  // K:   51    3   062          K 10   * Battery capacity (%)               100% ???
  // L:   55    4   0418         L 11   Inverter heat sink temperature (°C)   ????
  // M:   60    4   0000         M 12   * PV Input current for battery (A)   considerar que la bateria carga a 24V pero el panel da entre 30 y 80V
  // N:   65    5   000.0        N 13   * PV Input voltage 1 (V)
  // O:   71    5   24.92        O 14   * Battery voltage from SCC (V)          ???? Solar Charge Controller
  // P:   77    5   00001        P 15   * Battery discharge current (A)
  // Q:   83    8   10010000     Q 16   Device status (8bit)              mas detalle mas abajo DEVICE STATUS
  // R:   92    2   00           R 17   Battery  voltage  offset for fans on (10mV unidad)
  // S:   95    2   04           S 18   (4)  EEPROM version
  // T:   98    5   00000        T 19   * PV Charging power (W)
  // U:   104   3   000          U 20   (100) Inverterstatus  b10: flag for charging to floating mode  b9: Switch On    b8: flag  for  dustproof  installed(1-dustproof   installed,0-no  dustproof,  only  available  for HybridV series)

  int pos[21] = {1,7,12,18,23,28,33,37,41,47,51,55,60,65,71,77,83,92,95,98,104};
  int lar[21] = {5,4,5,4,4,4,3,3,5,3,3,4,4,5,5,5,8,2,2,5,3};
  String dato[21];
  int i;

  for(i=0; i<21; i++) {
    dato[i] = linea.substring(pos[i], pos[i]+lar[i]);   //captures data String
    Serial.println("dato[" + String(i) +"] substr(" + pos[i] + ", " + lar[i] + ") => " + dato[i]);
  }

 /*  
  separador = " "; // espacio en blanco
      pos2 = readString.indexOf(separador);  //finds location of first ,
      angle = readString.substring(0, pos2);   //captures first data String
      pos1 = pos2;
      pos2 = readString.indexOf(separador, pos2+1 );   //finds location of second ,
      fuel = readString.substring(pos1+1, pos2+1);   //captures second data String
      pos1 = pos2;
      pos2 = readString.indexOf(separador, pos2+1 );
      speed1 = readString.substring(pos1+1, pos2+1);
      pos1 = pos2;
      pos2 = readString.indexOf(separador, pos2+1 );
      altidude = readString.substring(pos1+1); //captures remain part of data after last ,
      pos1 = pos2;
*/
}



/*
//unsigned long SetTime = 0;

void tiempoIn(YYYYMMDDhhmmss) {
//            01234567890123
  // dias = totalSegundos / ( ss * mm * hh )
  anos = YYYYMMDDhhmmss.substring(0, 4) ;
  meses = YYYYMMDDhhmmss.substring(4, 2) ;
  dias = YYYYMMDDhhmmss.substring(6, 2) ;
  horas = YYYYMMDDhhmmss.substring(8, 2) ;
  minutos = YYYYMMDDhhmmss.substring(10, 2) ;
  segundos = YYYYMMDDhhmmss.substring(12, 2) ;

  totalSegundos = 0 ;  
  totalSegundos += anos * ( 60 * 60 * 24 * 30 * 12 ) ;
  totalSegundos += meses * ( 60 * 60 * 24 * 30 ) ;
  totalSegundos += dias * ( 60 * 60 * 24 ) ;
  totalSegundos += horas * ( 60 * 60 ) ;
  totalSegundos += minutos * ( 60 ) ;
  totalSegundos += segundos * ( 1 ) ;

  return totalSegundos ;
}

void tiempoOut(totalSegundos) {
  totalSegundos += (millis()/ 1000) ;
  // dias = totalSegundos / ( ss * mm * hh )
  anos = totalSegundos / ( 60 * 60 * 24 * 30 * 12 ) ;
  meses = totalSegundos / ( 60 * 60 * 24 * 30 ) ;
  dias = totalSegundos / ( 60 * 60 * 24 ) ;
  horas = totalSegundos / ( 60 * 60 ) ;
  minutos = totalSegundos / ( 60 ) ;
  segundos = totalSegundos / ( 1 ) ;

  return anos + "-" + meses + "-" + dias + " " + horas + ":" + minutos + ":" + segundos;
}
*/

  /*
   if(Serial.available()){
     Serial.write("-");
     Serial2.write(Serial.read());  
   }
   if(Serial2.available()){
     Serial.write(".");
     Serial.write(Serial2.read());  
   }
   */
