---
layout: post
title:  "Contador de visitas con Glitch"
date:   2020-06-04 13:21:35 -0500
author:   "Alejandro Alvarez"
categories: notas
---

Para hacer el contador de visitas de mi página web usé glitch. La idea era crear algo simple para saber si alguien visita mi sitio, buscando en internet encontré que lo más simple que podia hacer es una aplicación con JavaScript en glitch. Glitch es una herramienta para hacer aplicaciones web. Te permite hacer un aplicación con Node.js y publicarla en segundos.

Este contador de visitas no es nada robusto ni sofisticado, tampoco permite hacer un analisis muy profundo, pero sirve para saber si alguien visita mi página. Cada vez que alguien carga alguna de las páginas, se crea un registro con los siguientes datos:

- Dirección IP del visitante
- URL que visitó
- Fecha y hora de la visita

En Glitch escogí un proyecto nuevo con `hello-express`, es el más básico para crear aplicaciones web.
La estructura del proyecto recien creado se ve asi:

{% highlight c %}
assets
public/
    script.js
    style.css
views/
    index.html
.env
README.md
package.json
server.js
{% endhighlight %}

De ahí borré `public`, `views` y `README.md`, realmente lo único necesario es `packages.json`, `.env` y `server.js`.


Los frameworks que voy usar son:
- [Express](https://expressjs.com): Servidor web basado en Node.js.
- [Sqlite3 para JS](https://github.com/mapbox/node-sqlite3): Base de datos de los visitantes.
- [Sequelize](https://sequelize.org): Modelo de datos, no es taaaan necesario.

Primero que nada, añadí las dependencias a package.json:
{% highlight json %}
"dependencies": {
    "express": "^4.17.1",
    "sqlite3": "*",
    "sequelize": "*"
},
{% endhighlight %}

Y agregué el paquete de `sequelize` a `server.js`:
{% highlight js %}
var express = require("express");
var Sequelize = require('sequelize');
const { Op } = require("sequelize");

const app = express();
.
.
.
{% endhighlight %}

En `.env` definí las credenciales para la base de datos:
```
DB_USER=usr
DB_PASS=pwd
```

Lo que sigue es inicializar la base de datos en `server.js`, para eso borré todo desde abajo de `const app = express();` hasta el comentario `// listen for requests :)`, y lo reemplacé con lo siguiente:
{% highlight js %}
var Visitante;
var sequelize = new Sequelize('database', process.env.DB_USER, process.env.DB_PASS, {
  host: '0.0.0.0',
  dialect: 'sqlite',
  storage: '.data/database.sqlite' // Ruta donde guardaré la BD
});


sequelize.authenticate()
  .then(function(err) {
    console.log('Conexion exitosa');
    // Aqui definimos la estructura de la tabla
    Visitante = sequelize.define('visitante', {
        Direccion: { type: Sequelize.STRING },
        Pagina: { type: Sequelize.STRING }
    });
    // Adicional a esos campos Sequelize crea otros dos más:
    // updatedAt y createdAt de tipo DateTime

    Visitante.sync({force: false});
    // False para que al iniciar el servidor no se borre la
    // tabla si es que ya existe.
  }).catch(function (err) {
    console.log('Error al conectar a la base de datos: ', err);
  });
{% endhighlight %}

Ahora lo más importante es definir el comportamiento de la API, creé 4 rutas de HTTP para realizar lo siguiente:
{% highlight js %}
// Obtener lista de visitantes:
app.get("/", function (request, response) {});

// Crear registro de visitante:
app.get("/nueva", function (request, response) {});

// Obtener número de visitas:
app.get("/clicks", function (request, response) {});

// Obtener cantidad de direcciones IP diferentes:
app.get("/ubicaciones", function (request, response) {});
{% endhighlight %}

Lo que falta para que esté lista mi aplicación es llenar las queries de sequelize. Solo usé 3 tipos de queries diferentes:
{% highlight js %}
// Crear registro:
Visitante.create({ Direccion: ip, Pagina: pag});

// Contar registros:
Visitante.count({}).then(function(count){
    response.header("Access-Control-Allow-Origin", "*");
    response.send(count);
}

// Devolver lista de registros:
Visitante.findAll({}).then(function(visitantes){
    response.header("Access-Control-Allow-Origin", "*");
    response.send(visitantes);
}
{% endhighlight %}


Algunos puntos importantes:

- `response.header("Access-Control-Allow-Origin", "*");`: Permite llamar a la API desde otro dominio sin marcar error.
- `request['req']['headers']['x-forwarded-for'].split(",")[0];`: Obtiene la dirección IP de donde viene el *request*.
- `request['headers']['referer'];`: Viene la URL de donde se originó el *request*.


Por último, en cada página en la que quiero registrar los visitantes, agregue la llamada a mi nueva API usando JQuery:

{% highlight html %}
<script src="https://code.jquery.com/jquery-3.5.1.min.js"></script>
<script>
    $.get("https://<repo-de-glitch>.glitch.me/nueva");
</script>
{% endhighlight %}

Y si queremos desplegar el número de visitas y los visitantes en otra página podemos hacer lo siguiente, para llamar a la API con jQuery:
{% highlight html %}
<script src="https://code.jquery.com/jquery-3.5.1.min.js"></script>
<script>
    $(function(){
        $("#includedContent").load("https://<repo-de-glitch>.glitch.me/");
    });
    $(function(){
        $("#counts").load("https://<repo-de-glitch>.glitch.me/ubicaciones");
    });
    $(function(){
        $("#visitas").load("https://<repo-de-glitch>.glitch.me/clicks");
    });
</script>
{% endhighlight %}

Y para desplegarlo:
{% highlight html %}
<div class="container">
    <h2 id="visitas"></h2>
    <h2 id="counts"></h2>
    <br>
    <pre><code id="includedContent"></code></pre>
</div>
{% endhighlight %}

<h2>Resultados en tiempo real</h2>
<script src="https://code.jquery.com/jquery-3.5.1.min.js"></script>
<script>
    $(function(){
        $("#includedContent").load("https://achieved-golden-writer.glitch.me/");
    });
    $(function(){
        $("#counts").load("https://achieved-golden-writer.glitch.me/ubicaciones");
    });
    $(function(){
        $("#visitas").load("https://achieved-golden-writer.glitch.me/clicks");
    });
</script>
<div class="container">
    <div id="visitas"></div>
    <div id="counts"></div>
    <br>
    <pre><code id="includedContent"></code></pre>
</div>


La API ya terminada se ve asi:
<div class="glitch-embed-wrap" style="height: 420px; width: 100%;">
  <iframe
    src="https://glitch.com/embed/#!/embed/butter-jazzy-hearing?path=server.js&previewSize=0"
    title="butter-jazzy-hearing on Glitch"
    allow="geolocation; microphone; camera; midi; vr; encrypted-media"
    style="height: 100%; width: 100%; border: 0;">
  </iframe>
</div>
