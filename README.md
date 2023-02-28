# Intenet-Think-Inverter-AXpert-Monitor
Solucion para monitorear Inversor MPPT con Internet de las Cosas utilizando un ESP32 offline (solo LAN) o con Internet


Hace algunos años compre un Inversor MPPT AXpert de 3KV, este venia con un software hecho en JAVA muy malo, limitado, solo para PC por cable usb de impresora (inmaginense el MPPT en el patio y el PC haciendole compañia todo el dia), acumulaba datos solo cuando estaba conectado y no habia posibilidad de trabajar con los datos si no eran exportados por pequeños bloques

Desilucionante para un equipo caro que prometia mucho, en si, me inmagino a la gran mayoria de los usuarios haciendo uso de estos equipos sin obtener mas retroalimentacion que la que indica la pantalla del panel y unos pocos usuarios gastando una gran cantidad de dinero comprando una solucion intermedia que tambien debe tener sus condiciones y limitaciones, nada que te permita obtener la informacion que requieras y como la quieras

Desempleado, con tiempo, el conocimiento, el equipo a mi disposicion y la idea en la cabeza ...

Por esta razon dedique tiempo en desarrollar una solucion practica que hisiera todo lo que se me ocurriera, acumulara datos para post analisis, presentara informacion en directo de lo que estaba ocurriendo, pudiera ser util para analisis inmediato de datos anteriores y comparaciones graficas, etc... un mundo de ideas como que funcionara standalone sin Internet, como en una situacion offgrid real y que pudiera trabajar tambien con internet y aprovechar la capacidad de almacenamiento y computo de un servidor free

En resumen para no latear, lo logre, pero se perdio todo cuando el proveedor de servicio decidio borrar mi cuenta, no se las razones, ocupaba poco recurso y espacio, lo malo es que no solo se perdieron los datos, sino tambien las ultimas modificaciones que por razones de mobilidad se realizaron unicamente en el servidor y no las descargue 

Pero ire subiendo las versiones bases y la ultima que respalde en mi pc

DETALLE:

EL Inveror MPPT tiene una puerta SERIAL y una USB, se utilizo la serial para conectar el ESP32 leer los datos utilizando el protocolo que subire en PDF

La conexion del ESP32 es simple, solo el puerto serial RX/TX para comunicarse
Activar el Wifi para conectar en dos modos, AP (accessPoint) y CLI (cliente wifi), de esta forma en caso de estar en un sitio alejado de la ciudad y no contar con Internet, ni un Wifi, el ESP32 puede brindar una Wifi para conexion de interface propia y tambien para configuracion
Cuando trabaja offline en la EPROM se aloja los archivos del sitio web offline, tambien puede almacenar los datos y generar resumenes en una memoria externa tipo SD, para luego rescatarlos o revisarlos por medio de la interface web
Cuando trabaja online con internet se envian todos los datos a un URL donde un servidor alojara los datos, tambien recoje de este los datos historicos para analisis y grafica

En si parece simple, pero requirio mucha investigacion y prueba de la conexion su protocolo y la implementacion tanto offline como online y grafica fue constante mejora
Los datos acumulados hasta antes de ser eliminados por el proveedor eran de algo mas de un año, un registro por cada segundo, registrando el 100% de los datos y calculos intermedios del MPPT

Subire algunas fotos y capturas de pantalla para complementar a medida que me de el tiempo 

Esta solucion es mejor que muchas soluciones comerciales y muy economica

Esto me permitio conocer en detalle el comportamiento del MTTP, las baterias, los paneles, su limpieza, inclinacion, sombras, comportamiento ante amanecer, dias nublados, atardeceres, etc..
Realice multiples pruebas y el software me permitio analizar con datos certeros e inmediatos el comportamiento y sacar rendimiento al limite del equipo sin generarle daños

Debo aclarar que el MPPT solo duro un año y algo antes que comenzara a fallar por problemas de ventilador, y principalmente por oxidacion de componentes, estos equipos realmente no estan hechos para el interperie, menos aun para el campo, el calor los mata, la humedad los mata, el polvo los mata, en si, se requiere una placa mejor preparada y componentes mejor resguardados, no sirve tanta inversion por un solo año, tras la reparacion no lo e vuelto a conectar ya que, es demasiada preocupacion para un ahorro menor, la verdad no vale la pena si ya cuentas con energia electrica, solo si estas en el campo
El equipo MPPT requiere ser resguardado, estar en una habitacion, lejos del polvo, humedad, calor, pero aun asi van a encontrar muchos mensajes de falla de los ventiladores, que a pesar que son relativamente economicos, muchas veces reemplazarlos no implica una solucion, al parecer hay un codigo entre medio

El ESP32 solo lo meti en una cajita plastica y nunca dio problema, costo bajo, simple, rapido, bajo consumo, 100% la mejor herramienta para todo tipo de proyecto Internet de las cosas

El Sitio Web, es un sitio que utiliza API RESP que lo hace mas ligero en la comunicacion, rapido y fluido

Espero continuar subiendo los avance que estoy recolectando en mi PC