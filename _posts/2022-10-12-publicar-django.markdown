---
layout: post
title:  "Publicar proyecto de Django con Nginx"
date:   2022-10-22 18:00:00 -0500
author:   "Alejandro Alvarez"
categories: notas
---

Esta es una guia para publicar una aplicación de Django en un servidor de Linux, usando Nginx y Gunicorn. 

Primero que nada, esta guía esta pensada para CentOS 7, RHEL 7 o Oracle Linux 7. Para otras versiones de Linux algunos de los comandos pueden cambiar pero en escencia es lo mismo.

Para servir aplicaciones de Django, se usan 3 capas:
1. Un servidor web de archivos, a mi me gusta usar __Nginx__.
2. Un servidor web de Python, __Gunicorn__ soporta aplicaciones de WSGI, que es lo que usa Django.
3. Nuestra aplicación de __Django__.

__Nginx__ queda como punto de contacto al exterior, donde se reciben todas las peticiones de los clientes y redirige aquellas peticiones que llaman a nuestra aplicación web. __Gunicorn__ las recibe y las traduce a un formato que Django pueda entender y ejecutar. En general, Gunicorn no esta preparado para ser un servidor expuesto, sin embargo es necesario para poder ejecutar aplicaciones de Python. Nginx es una capa extra de seguridad y nos da funcionalidades para mejorar el rendimiento. 

{:refdef: style="text-align: center;" }
![My Image](/notas/assets/nginx-django/diagrama1.png){:width="100%"}
{: refdef}

Entre las cosas que Gunicorn no puede hacer:
- Servir https.
- Balancear de cargas.
- Manejar multiples dominios.
- Manejar diferentes servicios (Python, Php, ...).
- Contestar a muchas peticiones muy rápido.

Razones por las que se necesita Gunicorn:
- Nginx no puede ejecutar código de Python.

## Instalar nginx

Para instalar Nginx hay que configurar el repo de donde se va a descargar. Creamos un archivo en `/etc/yum.repos.d/nginx.repo` con el contenido siguiente:
{% highlight bash %}
[nginx]
name=nginx repo
baseurl=http://nginx.org/packages/mainline/centos/7/$basearch/
gpgcheck=0
enabled=1
{% endhighlight %}

Y para instalarlo usamos yum:
{% highlight bash %}
yum install nginx
{% endhighlight %}

Despues podemos iniciar Nginx para ver que todo funcione:
{% highlight bash %}
systemctl start nginx
systemctl status nginx
{% endhighlight %}

El estatus de Nginx se debe ver algo asi:
{% highlight bash %}
● nginx.service - The nginx HTTP and reverse proxy server
   Loaded: loaded (/usr/lib/systemd/system/nginx.service; enabled; vendor preset: disabled)
   Active: active (running) since Sat 2022-10-01 16:08:57 UTC; 2 weeks 3 days ago
 Main PID: 2922 (nginx)
   CGroup: /system.slice/nginx.service
           ├─ 2922 nginx: master process /usr/sbin/nginx
           ├─18300 nginx: worker process
           └─18301 nginx: worker process
{% endhighlight %}

Sí todo esta en orden, podemos apagarlo:
{% highlight bash %}
systemctl stop nginx
{% endhighlight %}

## Instalar Python

Por lo general en casi todas las distribuciones de Linux 7, viene un Python incluido. Si quisieramos instalar una version más reciente, hay que compilarla. 

Lo primero es descargar las herramientas:
{% highlight bash %}
sudo yum groupinstall "Development Tools" -y
yum install openssl-devel libffi-devel bzip2-devel wget
{% endhighlight %}

Despues hay que descargar y descomprimir el código de Python:
{% highlight bash %}
wget https://www.python.org/ftp/python/3.9.15/Python-3.9.15.tgz
tar -xvf Python-3.9.15.tgz
{% endhighlight %}

Para checar que todo este bien en nuestro sistema y configurar las herramientas antes de compilar, hay un script en la carpeta:
{% highlight bash %}
cd Python-3.9.15
./configure --enable-optimizations
{% endhighlight %}
  
En este paso es importante revisar que todo este correcto. Hay ocasiones en que el script no encuentra la biblioteca de SSL y ese caso no se podrán usar las librerias de SSL de Python. En ese caso, manualmente podemos apuntar a donde tenemos SSL con esta bandera:
{% highlight bash %}
./configure --with-openssl=/ruta/a/openssl
{% endhighlight %}

Finalmente para compilar todo y instalar los binarios de Python:
{% highlight bash %}
make altinstall
{% endhighlight %}

Y podemos verificar la instalacion:
{% highlight bash %}
# python3.9 --version
Python 3.9.15
{% endhighlight %}

Opcionalmente podemos decirle a Linux que esta va a ser nuestra version de Python por default, usando alternatives:
{% highlight bash %}
alternatives --set python3 /usr/bin/python3.9
{% endhighlight %}

## Instalar aplicacion
Usar git para nuestros proyectos hace las cosas más sencillas, sobre todo para mandar actualizaciones despues de la instalación. Pero bueno, por ahora sólo es copiar el proyecto a la carpeta de destino en el servidor:

{% highlight bash %}
# Si estamos usando git:
git clone ssh://git@<servidor-git>:/proyecto.git /opt/

# Si no, podemos copiarlo con scp:
scp -R /ruta/local/proyecto/ usuario@<ip-servidor>:/opt/


{% endhighlight %}
En ambos casos nuestro proyecto lo tendremos en la ruta `/opt/proyecto`.

Lo siguiente es instalar las dependencias necesarias para que el proyecto corra. Yo recomiendo usar ambientes virtuales por si en algún futuro hay que instalar más cosas en este servidor sin tener conflictos.

{% highlight bash %}
cd /opt/proyecto
python3 -m venv .env
source .env/bin/activate

# Si tenemos un archivo de pip
pip install -r requirements.txt

# Si no podemos instalar cada una de las dependencias
pip install django==X.X.X mysqlclient==X.X.X ...

{% endhighlight %}

Teniendo todo instalado, podemos configurar nuestro proyecto para publicarlo:

- Cambiar la llave de Django. Esto se hace en el archivo de `settings.py` del proyecto, hay una variable que se llama `SECRET_KEY`, esta llave debe de ser privada, es recomendable cambiarla al hacer la instalación del proyecto en un servidor de producción. 

- Configurar archivos estaticos, tambien en `settings.py`. La idea de esto es que Nginx sea capaz de servir los archivos estáticos sin tener que pasar por Gunicorn y Django. Lo que hacemos es decirle a Django que ponga todos los archivos estáticos en una carpeta a la que Nginx apunte. La configuración que yo pongo es la siguiente:

{% highlight python %}
# Esta primer variable indica en que url se puede acceder a archivos estaticos.
STATIC_URL = '/static/'

# Esta variable indica a Django donde va a colocar los archivos estaticos.
STATIC_ROOT = "/var/www/static/"

# Por ultimo, aqui se indica que carpetas en el proyecto contienen archivos estaticos.
STATICFILES_DIRS = (
    os.path.join(BASE_DIR, 'static'),
)
{% endhighlight %}

- Desabilitar el modo debug y configurar los hosts permitidos para CSRF. Simplemente es pasar la variable de `DEGUG` a `False` en `settings.py`. Y para `ALLOWED_HOSTS`, se necesita una lista con los dominios/servidores en los que Django va a servir. Para más detalles de esta configuracion, dejo un link [aqui](https://docs.djangoproject.com/en/4.1/ref/settings/#std-setting-ALLOWED_HOSTS).

- Instalar y probar Gunicorn.
{% highlight bash %}
pip install gunicorn

gunicorn -w 4 proyecto.wsgi:application
{% endhighlight %}

Despues de esto, si Gunicorn no muestra ningún error, podemos cerrarlo con _control c_ y salirnos del ambiente virtual de Python con el comando `deactivate`.

## Configurar Gunicorn

Ahora vamos a poner a Gunicorn a correr como un servicio de `systemd`, y que le sirva a Nginx en un socket que el usuario nginx pueda usar.

Primero hay que cambiar al dueño del proyecto, para que todo lo pudiera leer Nginx sin problemas:
{% highlight bash %}
chown -R nginx:nginx /opt/proyecto/
{% endhighlight %}

Despues hay que crear las carpetas para los archivos estáticos y copiar los archivos a esa ruta:
{% highlight bash %}
mkdir -p /var/www/static
chown -R nginx:nginx /var/www/ 
sudo -u nginx /opt/proyecto/.env/bin/python manage.py collectstatic
{% endhighlight %}

Por último hay que crear un archivo en `systemd` donde indiquemos como se ejecuta el proyecto con Gunicorn, ese archivo debe de ir en la ruta `/etc/systemd/system/` y en este caso lo nombraremos `proyecto.service`.


El archivo se debe de ver algo asi:

{% highlight bash %}
[Unit]
Description=Servidor de Gunicorn para Proyecto
After=network.target

[Service]
User=nginx
WorkingDirectory=/opt/proyecto
ExecStart=/opt/proyecto/.env/bin/gunicorn -t 0 --access-logfile /opt/proyecto/access_file_g.log --error-logfile /opt/proyecto/error_file_g.log --capture-output --enable-stdio-inheritance --workers 3 --bind unix:/opt/proyecto/gunicorn.sock proyecto.wsgi:application

[Install]
WantedBy=multi-user.target
{% endhighlight %}


Una vez agregado el archivo hay que recargar el daemon de `systemd` con el comando `systemctl daemon-reload`. 

Para activarlo y que siempre este corriendo:
{% highlight bash %}
systemctl enable proyecto
systemctl start proyecto
{% endhighlight %}

Y verificamos que todo este bien con:
{% highlight bash %}
systemctl status proyecto
{% endhighlight %}

## Configurar Nginx
De manera predeterminada, Nginx viene configurado para servir archivos de `/var/www`. Por lo tanto hay configurarlo para que se comunique con el proceso de Gunicorn a través del socket que creamos. 
Antes que nada vamos a la configuración general de Nginx en `/etc/nginx/nginx.conf`. Ahi vamos a quitar todos los bloques de `server` y vamos a añadir una linea para que lea esos bloques de server de una carpeta a la que vamos a llamar `sites-enabled`. El archivo debe de quedar algo asi:
{% highlight bash %}
# For more information on configuration, see:
#   * Official English Documentation: http://nginx.org/en/docs/
user nginx;
worker_processes auto;
error_log /var/log/nginx/error.log;
pid /run/nginx.pid;

# Load dynamic modules. See /usr/share/doc/nginx/README.dynamic.
include /usr/share/nginx/modules/*.conf;

events {
    worker_connections 1024;
}

http {
    log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
                      '$status $body_bytes_sent "$http_referer" '
                      '"$http_user_agent" "$http_x_forwarded_for"';

    access_log  /var/log/nginx/access.log  main;

    sendfile            on;
    tcp_nopush          on;
    tcp_nodelay         on;
    keepalive_timeout   65;
    types_hash_max_size 4096;

    include             /etc/nginx/mime.types;
    default_type        application/octet-stream;

    # Load modular configuration files from the /etc/nginx/conf.d directory.
    # See http://nginx.org/en/docs/ngx_core_module.html#include
    # for more information.
    include /etc/nginx/conf.d/*.conf;
    include /etc/nginx/sites-enabled/*;
}
{% endhighlight %}

Nótese que en la penúltima linea es donde agregamos la opción para que se carguen archivos de configuración de la carpeta `sites-enabled`. Por lo tanto hay que crear esa carpeta y de una vez creamos otra más:
{% highlight bash %}
mkdir /etc/nginx/sites-enabled/
mkdir /etc/nginx/sites-available/
{% endhighlight %}

En la carpeta de `sites-available/` vamos a tener un archivo de configuracion por cada servidor virtual que tengamos en Ngnix. En la carpeta de `sites-enabled/` vamos a poner un link simbolico para cada uno de estos sitios cuando queramos que sean publicos. El chiste de esto es que si en algún momento queremos dar de baja algun servicio de nuestro servidor, baste con borrar ese link simbolico sin necesidad de borrar la configuración original, y en el momento que necesitemos levantar de nuevo ese servidor bastará con crear de nuevo el link.

Hay 3 cosas que hay que tener en cuenta en esta configuración:
1. La direccion o dominio y puerto donde queremos que este nuestro proyecto.
2. La ubicacion del socket.
3. La forma en que se redirigen las peticiones a Gunicorn.

Teniendo esa información a la mano podemos crear un archivo de configuracion de Nginx en `/etc/nginx/sites-available/proyecto.site`:
{% highlight bash %}
server {    
    server_name xxx.xxx.xx.xxx dominio.com;

    location /static/ {
     root /var/www;    
    }

    location / {
     proxy_set_header Host $http_host;
     proxy_set_header X-Real-IP $remote_addr;
     proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
     proxy_set_header X-Forwarded-Proto $scheme;
     proxy_pass http://unix:/opt/proyecto/gunicorn.sock;
    }
    
    listen 80;
}
{% endhighlight %}

Con esto le estamos diciendo a Nginx que actue como _reverse proxy_ de Gunicorn y que mande a Gunicorn las peticiones que van a `/` de nuestro dominio y dirección IP. Si tuvieramos más aplicaciones en el mismo servidor, podriamos hacer que Nginx decida a donde redirigir las peticiones ya sea por dominio o por la ruta que se esta pidiendo.

Por último hay que poner nuestro sitio en `sites-enabled`:
{% highlight bash %}
ln -s /etc/nginx/sites-available/proyecto.site /etc/nginx/sites-enabled/proyecto.site
{% endhighlight %}

Una vez hecho esto, probamos que la configuración de Nginx sea válida con `nginx -t`, sí la configuración es válida iniciamos el servidor:
{% highlight bash %}
systemctl enable nginx
systemctl start nginx
{% endhighlight %}

## Configurar SELinux
Ya que esta configurado Nginx, al tratar de acceder a nuestra aplicacion es probableque veamos un error 502 BAD GATEWAY. Esto es causado cuando Nginx no se puede conectar con el servidor de Gunicorn. A partir de la version 6.6, RHEL y otras distribuciones derivadas de RHEL (como CentOS y Oracle Linux) vienen con una utilidad que se llama SELinux (Security-Enhanced Linux). SELinux bloquea algunas de las funcionalidades de los programas con motivos de seguridad. En este caso prohibe a Nginx (o cualquier servidor httpd) de escribir en algun en algun socket.

El error que vamos a ver cuando SELinux bloquea a Nginx primero se puede ver en `/var/log/nginx/error.log` con un mensaje parecido a este:
{% highlight bash %}
2022/10/19 19:35:51 [crit] 31398#31398: *9404 connect() to unix:/opt/proyecto/gunicorn.sock failed (13: Permission denied) while connecting to upstream, client: xxx.xxx.xxx.xxx, server: 35.xxx.xxx.xxx, request: "GET / HTTP/1.1", upstream: "http://unix:/opt/proyecto/gunicorn.sock:/", host: "35.xxx.xxx.xxx"
{% endhighlight %}

Aun que este error podria ser causado por diferencias entre el usuario que ejecuta nginx y el usuario que ejecuta Gunicorn. Al asegurarnos que Nginx y Gunicorn corren como el mismo usuario podemos descartar esa posibilidad:
{% highlight txt %}
# ps -aux
> USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
> root      2922  0.0  0.1  40100  1444 ?        Ss   Oct01   0:00 nginx: master process /usr/sbin/nginx
> nginx    31397  0.0  0.1  40104  1560 ?        S    19:08   0:00 nginx: worker process
> nginx    31398  0.0  0.1  40104  1712 ?        S    19:08   0:00 nginx: worker process
.
.
.
> nginx    31584  0.0  1.5 218260 15552 ?        Ss   19:35   0:00 /opt/proyecto/env/bin/python3 /opt/proyecto/env/bin/gunicorn
> nginx    31587  0.1  3.2 278220 33092 ?        S    19:35   0:00 /opt/proyecto/env/bin/python3 /opt/proyecto/env/bin/gunicorn
> nginx    31588  0.0  3.2 278220 33004 ?        S    19:35   0:00 /opt/proyecto/env/bin/python3 /opt/proyecto/env/bin/gunicorn
> nginx    31589  0.0  3.2 278220 33044 ?        S    19:35   0:00 /opt/proyecto/env/bin/python3 /opt/proyecto/env/bin/gunicorn
{% endhighlight %}

Por ultimo, si verificamos la bitacora de SELinux en `/var/log/audit/audit.log` podemos encontrar la causa del error:
{% highlight txt %}
type=AVC msg=audit(1666208151.375:278085): avc:  denied  { write } for  pid=31398 comm="nginx" name="gunicorn.sock" dev="sda2" ino=17760691 scontext=system_u:system_r:httpd_t:s0 tcontext=system_u:object_r:user_home_t:s0 tclass=sock_file permissive=0
type=SYSCALL msg=audit(1666208151.375:278085): arch=c000003e syscall=42 success=no exit=-13 a0=8 a1=56293afc6200 a2=6e a3=7fffb1df8e40 items=0 ppid=2922 pid=31398 auid=4294967295 uid=997 gid=995 euid=997 suid=997 fsuid=997 egid=995 sgid=995 fsgid=995 tty=(none) ses=4294967295 comm="nginx" exe="/usr/sbin/nginx" subj=system_u:system_r:httpd_t:s0 key=(null) 
{% endhighlight %}

Posibles soluciones:
1. Desactivar SELinux con el comando `setenforce 0`.
2. Permitir a los procesos de tipo httpd_t como nginx correr en modo permisivo con el comando `semanage permissive -a httpd_t`.
3. Crear una regla que permita a Nginx leer y escribir en sockets usando `audit2allow`:
{% highlight txt %}
# grep nginx /var/log/audit/audit.log | audit2allow -m nginx > nginx.te
# cat nginx.te

module nginx 1.0;

require {
	type httpd_t;
	type user_home_t;
	class sock_file write;
}

#============= httpd_t ==============
allow httpd_t user_home_t:sock_file write;
{% endhighlight %}
Para usar es politica es necesario compilarla con estos dos comandos:

{% highlight txt %}
checkmodule -M -m -o nginx.pp nginx.te
semodule_package -m nginx.mod -o nginx.pp
{% endhighlight %}

Y despues instalarla:
{% highlight txt %}
semodule -i nginx.pp
{% endhighlight %}

Probablemente la tercera opcion sea la mas segura, pero cualquiera de las tres funciona. Para solucionar otros problemas con Nginx y SELinux pueden ver la documentacion de Nginx [aqui](https://www.nginx.com/blog/using-nginx-plus-with-selinux/).
