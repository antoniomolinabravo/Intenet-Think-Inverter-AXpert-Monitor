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
//  CAPA JAVASCRIPT 
//  RECIBE MENSAJE HTTP DESDE EL SERVIDOR
//  PARCEA EL STRING ENVIADO POR EL ESP32  QUE CORRESPONDE A LA RESPUESTA DEL INVERTER
//  REALIZA CALCULOS Y REGISTRA HISTORICOS PARA ESTIMACIONES
//  PRESENTA DATOS VIA DOM HTML
//


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

var spanOutDebug = document.getElementById("OutDebug");

function parce_and_set(linea){
var tmp = [];
var arr = [];
 tmp = linea.split('(');
 arr = tmp[1].split(' ');
 //dato1 = arr[0];
 //dato2 = arr[1];
 //dato3 = arr[2];
 //dato4 = arr[3];
 //dato5 = arr[4];
 console.table(arr);
 return arr;
}

function outDebug(msg){
	spanOutDebug.innerText += msg;
}

// Create events for the sensor readings
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('gyro_readings', function(e) {
    console.log("gyro_readings", e.data);
    //var obj = JSON.parse(e.data);
    //document.getElementById("gyroX").innerHTML = obj.gyroX;
    //document.getElementById("gyroY").innerHTML = obj.gyroY;
    //document.getElementById("gyroZ").innerHTML = obj.gyroZ;

    // Change cube rotation after receiving the readinds
    //cube.rotation.x = obj.gyroY;
    //cube.rotation.z = obj.gyroX;
    //cube.rotation.y = obj.gyroZ;
    //renderer.render(scene, camera);
  }, false);

  source.addEventListener('PIGS', function(e) {
    console.log("PIGS", e.data);
    outDebug("PIGS: " + e.data);
	var arr = [];
	arr = parce_and_set(e.data);
    document.getElementById("A").innerHTML = arr[0];
    document.getElementById("B").innerHTML = arr[1];
  }, false);

  source.addEventListener('accelerometer_readings', function(e) {
    console.log("accelerometer_readings", e.data);
    //var obj = JSON.parse(e.data);
    //document.getElementById("accX").innerHTML = obj.accX;
    //document.getElementById("accY").innerHTML = obj.accY;
    //document.getElementById("accZ").innerHTML = obj.accZ;
  }, false);
}

function resetPosition(element){
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/"+element.id, true);
  console.log(element.id);
  xhr.send();
}

function sendCmd(id, value){
  var xhr = new XMLHttpRequest();

xhr.addEventListener("readystatechange", () => {
  if (xhr.readyState === 4 && xhr.status === 200)
    console.log("RESP("+id+":"+value+"):"+xhr.responseText);
});

  xhr.open("GET", "/cmd?"+id+"="+value, true);
  console.log("/cmd?"+id+"="+value);
  xhr.send();
}