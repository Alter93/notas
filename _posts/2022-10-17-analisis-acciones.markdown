---
layout: post
title:  "Analisis de datos de la bolsa con Python"
date:   2022-10-31 23:59:59 -0500
author:   "Alejandro Alvarez"
categories: notas
---

Esta es una guia para publicar una aplicacion de Django en un servidor de Linux, usando Nginx y Gunicorn. 

Primero que nada, esta guia esta pensada para CentOS 7, RHEL 7 o Oracle Linux 7. Para otras versiones de Linux, algunos de los comandos pueden cambiar pero en escencia es lo mismo.

Para servir aplicaciones de Django, se usan 3 capas:
1. Un servidor web de archivos, a mi me gusta usar __Nginx__.
2. Un servidor web de Python, __Gunicorn__ soporta aplicaciones de WSGI, que es lo que usa Django.
3. Nuestra aplicacion de __Django__.

__Nginx__ queda como punto de contacto al exterior, donde se reciben todas las peticiones de los clientes y redirige aquellas peticiones que llaman a nuestra aplicacion web. __Gunicorn__ recibe estas peticiones y las traduce a un formato que Django pueda entender y ejecutar. Nginx actua como una capa extra de seguridad y tambien da funcionalidad para mejorar el rendimiento. En general, Gunicorn no esta preparado para ser un servidor expuesto, sin embargo es necesario para poder ejecutar aplicaciones de Python.

{:refdef: style="text-align: center;" }
![My Image](/notas/assets/nginx-django/diagrama1.png){:width="100%"}
{: refdef}

Entre las cosas que Gunicorn no puede hacer:
- Servir https.
- Balancear de cargas.
- Manejar multiples dominios.
- Manejar diferentes servicios (Python, Php, ...).
- Contestar a muchas peticiones muy rapido.

Razones por las que se necesita Gunicorn:
- Nginx no puede ejecutar codigo de Python.

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
a
{% highlight bash %}
yum install nginx
{% endhighlight %}

## Instalar Python
## Instalar aplicacion
### Instalar dependencias
### Preparar archivos estaticos
### Configurar Gunicorn
### Cambiar permisos

## Configurar Nginx
## Configurar Selinux
## Configurar firewall

