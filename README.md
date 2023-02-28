# Intenet-Think-Inverter-AXpert-Monitor
Solucion para monitorear Inversor MPPT con Internet de las Cosas utilizando un ESP32 offline (solo LAN) o con Internet


Hace algunos años compre un Inversor MPPT AXpert de 3KV, este venia con un software hecho en JAVA muy malo, limitado, solo para PC por cable usb de impresora (imaginense el MPPT en el patio y el PC haciendole compañia todo el dia), acumulaba datos solo cuando estaba conectado y no habia posibilidad de trabajar con los datos si no eran exportados por pequeños bloques

Desilucionante para un equipo caro que prometia mucho, en si, me imagino a la gran mayoria de los usuarios haciendo uso de estos equipos sin obtener mas retroalimentacion que la que indica la pantalla del panel y unos pocos usuarios gastando una gran cantidad de dinero comprando una solucion intermedia que tambien debe tener sus condiciones y limitaciones, nada que te permita obtener la informacion que requieras y como la quieras

... Yo desempleado, con tiempo, el conocimiento, el equipo a mi disposicion y la idea en la cabeza ...

Me dedique de lleno a desarrollar una solucion practica que funcionara en pc y telefono, desde cualquier lugar, que hisiera todo lo que se me ocurriera, acumulara datos para post analisis, precalculos para una mejor comprension, presentara informacion en directo de lo que estaba ocurriendo, con una frecuencia de 1 segundo, pudiera ser util para analisis inmediato de datos anteriores y comparaciones graficas, etc... un mundo de ideas como que funcionara standalone sin Internet, como en una situacion offgrid real y que pudiera trabajar tambien con internet y aprovechar la capacidad de almacenamiento y computo de un servidor free

En resumen para no latear, lo logre, pero se perdio todo o casi todo, cuando el proveedor de servicio decidio borrar mi cuenta, no se las razones, ocupaba poco recurso y espacio, lo malo es que no solo se perdieron los datos, sino tambien las ultimas modificaciones que por razones de mobilidad se realizaron unicamente en el servidor y no las descargue 

No me gusta hacer las cosas dos veces, pero ...
Hoy me animo a recuperar el trabajo perdido ...
Por eso ire subiendo las versiones bases y la ultima que respalde en mi pc

DETALLE:

EL Inveror MPPT tiene una puerta SERIAL y una USB, se utilizo la serial para conectar el ESP32 leer los datos utilizando el protocolo que subire en PDF

La conexion del ESP32 es simple, solo el puerto serial RX/TX para comunicarse.
Activar el Wifi para conectar en dos modos, AP (accessPoint) y CLI (cliente wifi), de esta forma en caso de estar en un sitio alejado de la ciudad y no contar con Internet, ni un Wifi, el ESP32 puede brindar una Wifi para conexion de interface propia y tambien para configuracion.
Cuando trabaja offline en la EPROM se aloja los archivos del sitio web offline, tambien puede almacenar los datos y generar resumenes en una memoria externa tipo SD, para luego rescatarlos o revisarlos por medio de la interface web.
Cuando trabaja online con internet se envian todos los datos a un URL donde un servidor alojara los datos, tambien recoje de este los datos historicos para analisis y grafica.

La solucion diseñada para presentarse bonito, rapido, inmediato, claro y fluido, en PC y telefono

En si parece simple, pero requirio mucha investigacion y prueba de la conexion su protocolo y la implementacion tanto offline como online y grafica fue constante mejora.

Conocimentos y habilidades puestas en practica: programacion de microcontrolador en lenguaje C, electronica, comunicacion serial, ingenieria inversa de protocolos, html, javascript, API RESP, SQL, administracion de servidor web y base de datos, control de sesiones wifi desde microcontrolador, estadisticas, electricidad, instalacion de paneles solares, baterias, entre otras. 

Queda agregar que esto incluyo la modificacion del tablero electrico de la casa, prepara uno propio para el MPPT, cablear paneles, baterias, tierra, otros dos monitoreos independientes al MPPT y el ESP32 que fueron instalados en uno en la linea de conexion y otro en el tablero, asi se verificaron datos tanto de la produccion de los paneles, generacion por parte del MPPT, carga de bateria, entrega de carga, consumo de la casa. etc.

Los datos acumulados hasta antes de ser eliminados por el proveedor eran de algo mas de un año, un registro por cada segundo, registrando el 100% de los datos y calculos intermedios del MPPT.

Subire algunas fotos y capturas de pantalla para complementar a medida que me de el tiempo 

Esta solucion es mejor que muchas soluciones comerciales y muy economica

Esto me permitio conocer en detalle el comportamiento del MTTP, las baterias, los paneles, su limpieza, inclinacion, sombras, comportamiento ante amanecer, dias nublados, atardeceres, etc..

Realice multiples pruebas y el software me permitio analizar con datos certeros e inmediatos el comportamiento y sacar rendimiento al limite del equipo sin generarle daños

Debo aclarar que el MPPT solo duro un año y algo antes que comenzara a fallar por problemas de ventilador, y principalmente por oxidacion de componentes, estos equipos realmente no estan hechos para el interperie, menos aun para el campo, el calor los mata, la humedad los mata, el polvo los mata, en si, se requiere una placa mejor preparada y componentes mejor resguardados, no sirve tanta inversion por un solo año, tras la reparacion no lo he vuelto a conectar, ya que, es demasiada preocupacion para un ahorro menor, la verdad no vale la pena si ya cuentas con energia electrica de red, solo si estas en el campo o si sufres de frecuentes cortes de energia, en mi caso queria ahorrar dinero, brindar continuidad energetica y considerando que me dedicaria a la impresion 3D, esto era una buena alternativa, y claro el medio ambiente tambien. .. .y la curiosidad por su puesto .. fueron las razones que me llevaron a embarcarme en esto. 

El equipo MPPT requiere ser resguardado, estar en una habitacion, lejos del polvo, humedad, calor, pero aun asi van a encontrar muchos mensajes de falla de los ventiladores, que a pesar que son relativamente economicos, muchas veces reemplazarlos no implica una solucion, al parecer hay un codigo entre medio

El ESP32 solo lo meti en una cajita plastica y nunca dio problema, costo bajo, simple, rapido, bajo consumo, 100% la mejor herramienta para todo tipo de proyecto Internet de las cosas

El Sitio Web, es un sitio que utiliza API RESP que lo hace mas ligero en la comunicacion, rapido y fluido

Espero continuar subiendo los avance que estoy recolectando en mi PC
Y las experiencias que pueda compartiles con la solucion fotovoltaica
