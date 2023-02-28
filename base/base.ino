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
// Retorna estados 1 habilitado, 0 desabilitado
//
// QMCHGCR: 51-4D-43-48-47-43-52-D8-55-0D <-- rs:(020 030 040 050 060
// Solicita rangos de operacion
// Retorna rangos
// ----------------------------------------------------------------------------------------------------------------------------------------------------



#include <Arduino.h>

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

void setup() {
   Serial.begin(115200);
   Serial2.begin(2400, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
int i=0;
int delta = 950;

  serialCmd(QPI);
  delay(delta);
  serialCmd(QVFW);
  delay(delta);
  serialCmd(QVFW2);
  delay(delta);
  serialCmd(QFLAG);
  delay(delta);
  serialCmd(QMOD);
  delay(delta);
  serialCmd(QPIRI);

  while(i<=15) {
    i++
    delay(delta);
    serialCmd(QPIGS);
  } i=0;
}

void serialCmd(command) {
  String stringOne = "";
  Serial2.print(command);
  Serial.println("-> " + command);
  delay(30);
  stringOne = Serial2.readString();
  stringOne = "(001.0 00.0 228.0 50.0 0000 0000 000 345 24.19 000 062 0418 0000 000.0 24.92 000 01 10010000 00 04 00000 000";
  Serial.println ("<- " + stringOne );
}




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
