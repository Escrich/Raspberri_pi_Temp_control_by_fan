Para instalar el control del ventilador en tu raspberry, necesitas seguir los siguientes pasos:

Metodo largo, compilando:

cd $HOME

git clone https://github.com/Escrich/Raspberri_pi_Temp_control_by_fan.git

cd Raspberri_pi_Temp_control_by_fan/Source_code

gcc -Wall -o tempfanpwm tempfanpwm.c -lwiringPi -lpthread

sudo cp tempfanpwm /usr/bin/tempfanpwm 



Opcional:
Si quieres probar que funciona:

sudo ./tempfanpwm



Continuamos:

cd $HOME

sudo nano /etc/rc.local

Una vez aquí dentro, siempre por encima de la linea donde pone exit 0, escribimos las siguientes lineas:

#echo " Control automatico de ventilador en marcha "

echo

tempfanpwm &




salvamos con control o, y salimos con control x

reiniciamos la raspberry con:

sudo reboot now

Y al arrancar de nuevo, el control del ventilador, y el led intermitente, estarán funcionando

También encontrarás un log en el directorio /tmp/, que se llama ventilador.log

y que te proporciona información del estado del control del ventilador

Este metodo largo te permite tener el código fuente, (tempfanpwm.c), y hacer cambios si lo consideras oportuno.



Metodo corto, copiando el ejecutable que está en:
https://github.com/Escrich/Raspberri_pi_Temp_control_by_fan/blob/master/Compiled_executable_file/tempfanpwm

Baja este fichero y ponlo en tu maquina, por ejemplo en el directorio /home/pi, a partir de ahí ejecuta los siguientes comandos:

cd $HOME

sudo cp tempfanpwm /usr/bin/tempfanpwm 

sudo nano /etc/rc.local

Una vez aquí dentro, siempre por encima de la linea donde pone exit 0, escribimos las siguientes lineas:

#echo " Control automatico de ventilador en marcha "

echo

tempfanpwm &

salvamos con control o y salimos con control x

reiniciamos la raspberry con:

sudo reboot now

Y al arrancar de nuevo, el control del ventilador, y el led intermitente, estarán funcionando

También encontrarás un log en el directorio /tmp/, que se llama ventilador.log

y que te proporciona información del estado del control del ventilador





