Instalar y hacer cambios en el programa del ventilador:
-------------------------------------------------------

Si solo queremos poner una version nueva tras haber hecho cambios, lo demás está hecho, solo necesitamos compilar el fichero fuente que acabamos de descargar, en el que hemos hecho los cambios:

2 – Accedemos al directorio descargado:
---------------------------------------
cd Raspberry_PWM_FanTempControl

3 – Compilamos el código y generamos el archivo ejecutable:
-----------------------------------------------------------
gcc -o tempfanpwm tempfanpwm.c -lwiringPi -lpthread

gcc -o tempfan tempfanadv.c -lwiringPi -lpthread

Lo copiamos a donde están los ejecutables, antes hay que pararlo si está en ejecucion
sudo cp -a tempfanpwm /usr/bin/

4 – Ejecutamos el programa en segundo plano:
--------------------------------------------
sudo ./tempfanpwm &


5 – (Opcional) Para detener el programa:
----------------------------------------
PID=`sudo pidof tempfanpwm`
sudo kill -9 $PID


Hacer que el programa se lance al arrancar el sistema
-----------------------------------------------------
La forma más sencilla, y quizás la más apta para utilizar en la mayoría de distros, es la de lanzar el programa a través del archivo rc.local (el cual se ejecuta al arrancar el sistema y una vez que todos los servicios iniciales estén inicializados):

1 – Copiamos nuestro programa de control compilado en el directorio de binarios del usuario (con esto se consigue poder llamar a nuestro programa desde cualquier ruta, pues el directorio /usr/bin, al igual que otros, pertenecen a la llamada variable de entorno PATH):

sudo cp -a tempfanpwm /usr/bin/

2 – (Opcional) Una vez con el programa compilado y posicionado en /usr/bin, podemos borrar todo el directorio que contiene el archivo fuente:
---------------------------------------------------------------------------------------------------------------------------------------------
cd ..
sudo rm -rf Raspberry_PWM_FanTempControl

3 – Adquirimos privilegios de administración:
---------------------------------------------
sudo -i

4 – Añadimos al archivo rc.local el comando para lanzar a nuestro programa (se puede hacer editando el archivo, pero con los siguientes comandos nos aseguramos de que se hace de forma correcta, antes de la línea «exit 0»):

cp -a /etc/rc.local /etc/rc.local.back # Hacemos una copia de seguridad del archivo original
sed -i '/exit/d' /etc/rc.local # Eliminamos la linea exit 0
echo "tempfanpwm" >> /etc/rc.local # Insertamos la linea tempfan al final del archivo
echo "exit 0" >> /etc/rc.local # Insertamos la linea exit 0 al final del archivo


Hecho lo anterior, se consigue que cada vez que arranque la Raspberry se ejecute el programa de control del ventilador, para probarlo, basta con reiniciar el sistema:

reboot
