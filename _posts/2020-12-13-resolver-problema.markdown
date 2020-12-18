---
layout: post
title:  "Jugar a EuroMillones con Algoritmos Genéticos"
date:   2020-12-13 19:25:38 -0500
author:   "Alejandro Alvarez"
categories: notas
---

En un [artículo anterior hablé del algoritmo genético]({% post_url 2020-05-27-algoritmos-geneticos %}) y en esta ocasión vamos a resolver un problema usando ese algoritmo. El problema que elegí es como generar una combinación de números que sea _más probable_ de ganar EuroMillones.

<img src="/notas/assets/euromillones/algoritmo_genetico.png" alt="Individuo1" width="88%"/>

EuroMillones es una lotería que se juega en varios paises de Europa. El primer sorteo fue en el 2004 y cada semana hay 2 sorteos. Durante el sorteo se extraen 5 bolas de una máquina con 50 bolas númeradas seguido de 2 bolas que se extraen de otra máquina con 12 bolas numeradas. Por lo tanto, se juega con cinco números del 1 al 50 y dos números del 1 al 12 (estrellas de la suerte).

El objetivo de nuestro programa es encontrar una combianción de números que sea lo mas probable de ganar el sorteo usando los datos históricos de EuroMillones. Para lograr esto necesitamos lo siguiente:

1. Una forma de representar las posibles soluciones del problema. En este caso la representación esta dada por el problema: un arreglo de 7 números, los primeros cinco deben de estar entre 1 y 50 y no repetirse entre ellos, los últimos dos son números distintos entre el 1 y el 12.
2. Un función para evaluar nuestras soluciones. Sabiendo los datos historicos, ¿qué tan probable es que este grupo de números salga en el siguiente sorteo?
3. Operadores genéticos que vayamos a utilizar.

El programa lo escribiré en python tratando siempre hacerlo lo más simple posible. Empezamos por escribir el algoritmo genético de una forma semi-abstracta:

{% highlight python %}
# Inicializaciones
generaciones = T
tamaño_poblacion = N
poblacion = crear_poblacion(tamaño_poblacion)

# Búsqueda
for i in range(generaciones):
    soluciones_nuevas = []

    # Generar nueva población
    while len(soluciones_nuevas) < tamaño_poblacion:

        # Seleccionar padres
        (padre_1, padre_2) = seleccionar(poblacion)

        # Crear dos soluciones nuevas a partir de dos padres
        (hijo_1, hijo_2) = cruzar(padre_1, padre_2)

        # Mutar soluciones
        hijo_1 = mutar(hijo_1)
        hijo_2 = mutar(hijo_2)

        # Evaluar soluciones nuevas
        hijo_1.evaluar()
        hijo_2.evaluar()

        # Añadir hijos a población nueva si cumplen con
        # los requerimientos del problema
        if hijo_1.es_valido():
            soluciones_nuevas.append(hijo_1)
        if hijo_2.es_valido():
            soluciones_nuevas.append(hijo_2)

        # Reemplazar soluciones viejas con las nuevas
        poblacion = soluciones_nuevas

poblacion = ordenar(poblacion)
{% endhighlight %}

## Representación de la solución
Representar el juego de la lotería en un algoritmo genético es un problema sencillo, solamente hay que poner atención a 3 cosas:
- Cantidad de números con los que se juegan
- Valores mínimos y máximos
- Números repetidos

En EuroMillones se juega con 7 números:

$$
s = [ a, b, c, d, e, x, y ]
$$

Para $$ a,b,c,d,e $$ se deben seleccionar cinco números diferentes entre el 1 y el 50. Para $$x,y$$ se seleccionan dos números distintos entre 1 y el 12, a estos números se les llama estrellas. Por lo tanto para saber si una solucion es válida podemos escribir una función:

{% highlight python %}
def es_valido(solucion):
    valido = True

    # El tamaño de la solucion es la primer verificacion que debemos hacer:
    if len(solucion) != 7:
        valido = False

    # Limite inferior
    if any(y < 1 for y in solucion):
        valido = False

    # Dividimos nuestra solucion en dos:
    cinco_numeros = solucion[0:5]
    estrellas = solucion[5:]

    # Limite superior
    if any(y > 50 for y in cinco_numeros):
        valido = False

    if any(y > 12 for y in estrellas):
        valido = False

    # Verificar repetidos
    repetidos = [x for x in cinco_numeros if cinco_numeros.count(x) > 1]
    repetidos += [x for x in estrellas if estrellas.count(x) > 1]

    if len(repetidos) > 0:
        valido = False

    return valido

{% endhighlight %}

## Generar problación inicial
Sabiendo como vamos a representar los boletos del EuroMillones, podemos crear aleatoriamente soluciones que cumplan con los requerimientos explicados en la sección anterior.

Un forma simple de hacerlo es simulando la forma en la que sacan las pelotitas en un sorteo. tenemos un conjunto de 50 números y otro de 12, seleccionamos 5 del primer conjunto aleatoriamente y 2 del segundo conjunto.

{% highlight python %}
def crear_solucion():
    solucion = []

    # Dos conjuntos de posibles numeros
    conjunto_1 = list(range(1,51))
    conjunto_2 = list(range(1,13))

    # Seleccionamos 5 del primer conjunto
    for i in range(5):
        index = random.randint(0, len(conjunto_1)-1)
        print(len(conjunto_1))
        solucion.append(conjunto_1[index])
        # Eliminamos el elemento para evitar repetidos
        del conjunto_1[index]

    # Seleccionamos 2 del segundo conjunto
    for i in range(2):
        index = random.randint(0, len(conjunto_2)-1)
        solucion.append(conjunto_2[index])
        # Eliminamos el elemento para evitar repetidos
        del conjunto_2[index]

    return solucion
{% endhighlight %}


Para generar la población completa:
{% highlight python %}
def crear_poblacion(n):
    poblacion = []

    for i in range(n):
        poblacion.append(crear_solucion())

    return poblacion
{% endhighlight %}

## Evaluación de las soluciones
Esta es probablemente la parte más importante del programa. Saber que combinaciónes son más probables de ganar un sorteo es lo que va a guiar la búsqueda. Existen muchas teorías acerca de como ganar la lotería de los cuales se derivan diferentes formas para calcular las probabilidades. Yo no soy tan aficionado a los juegos de azar, pero tuve algunas ideas acerca de que combinaciones pueden ser mas probables de ganar.

1. Los números que se han repetido recientemente, probablemente se vuelvan a repetir.
2. Hay conjuntos de números que son improbables de salir seleccionados como por ejemplo 1,2,3,4,5,6,7.
3. Tomando en cuenta el punto anterior, podriamos decir que hay conjuntos de números que tienen a salir más.
4. De igual manera, es muy poco probable, que si ya salio un conjunto de números, este se repita en el corto plazo.

Usando estadistica Bayesiana podemos tratar de calcular la probabilidad de resulte ganador un conjunto de números. El teorema de Bayes propone un forma de calcular la probabilidad que ocurra un evento a partir de los resultados de algun otro evento. A esto se le conoce como probabilidad condicional.

Por lo tanto, la probabilidad de que ocurra un evento $$A$$, dado que ocurrió $$B$$, esta dado por:

$$
P(A|B) = \frac{P(B|A) * P(A)}{P(B)}
$$

Usando esa formula podriamos calcular cual es la probabilidad de que salga el número 2 despues de que salió el 1 en un sorteo:

$$
P(2|1) = \frac{P(1|2) * P(2)}{P(1)}
$$

Donde:

$$
P(1) = P(2) = \frac{1}{50}
$$

$$
P(1|2) = \frac{1}{50} * \frac{1}{49}
$$

Por lo tanto:

$$
P(2|1) = \frac{1}{2,450}
$$

Pero si tenemos una muestra de los 104 últimos juegos (los juegos de las últimas 52 semanas), podemos calcular todas las probabilidades condicionales usando parejas de números basando las probailidades en los números que más han salido juntos. Los datos históricos de EuroMillones se pueden bajar de [su página web](https://www.loterie-nationale.be/nos-jeux/euromillions/resultats-tirage) en formato CSV. Tomando los cinco números por aparte de las estrellas, hay 10 formas de agrupar esos 5 números en pares. La probabilidad de que salga un par de números seleccionados en un sorteo se puede estimar contando cuantas veces ha salido $$N_1$$, cuantas $$N_2$$ y cuantas veces han salido juntos:

$$
P(N_1,N_2) = \frac{\text{frecuencia}(N_1,N_2)}{\text{frecuencia}(N_2) + \text{frecuencia}(N_1)}
$$

Y si escogemos 5 números para un boleto de EuroMillones y queremos agrupar las probabilidades de que cada par de números salgan sorteados, podemos calcular el producto de la probabilidades de la siguiente forma:

$$
f(\text{Solución}) = \prod^{5}_{i = 1}{\prod^{5}_{j = i + 1}} P(N_i,N_j)
$$

A su vez, si sabemos que esos 5 números que esocogimos ya ha salido antes en otro sorteo, podemos calcular la probabilidad de que se repita ese escenario asi:

$$
P(S_1|S_1) = \frac{1}{2,118,760} \cdot P(S_1)
$$

Y podemos combinar ambas ecuaciones en una sola:

$$
G(s) =
\begin{cases}

    \frac{1}{2,118,760} \cdot \prod\prod P(N_i,N_j),& \text{si } s \in H\\
    \prod\prod P(N_i,N_j),              & \text{si } s \notin H
\end{cases}
$$

Donde $$H$$ es el conjunto de sorteos pasados.

Si transformamos esto a código:

{% highlight python %}
def evaluar_solucion(solucion):
    # Buscar todos los pares en la solución
    pares = pares_solucion(solucion)
    probabilidad = 1
    for par in pares:
        probabilidad = probabilidad * calcular_probabilidad_par(par)

    probabilidad = probabilidad * probabilidad_repetir(solucion)

    return probabilidad
{% endhighlight %}

Asumiendo que nuestro historico contiene dos matrices, una con frecuencias de cada número, y otra con las frecuencias por par de números. Podemos calcular las probabilidades de la siguiente forma:

{% highlight python %}
def calcular_probabilidad_par(par):
    probabilidad = 1/2450
    frecuencias_pares = historico["frecuencias_pares"]
    frecuencias = historico["frecuencias"]

    if (frecuencias_pares[par[0]][par[1]] != 0):
        n1_n2 = frecuencias_pares[par[0]][par[1]]

        n1 = frecuencias[par[0]]
        n2 = frecuencias[par[1]]

        probabilidad = (n1_n2 / (n1 + n2))

    return probabilidad
{% endhighlight %}

{% highlight python %}
def pares_solucion(solucion):
    pares = []
    for i in range(5):
        for j in range(i + 1, 5):
            par = [int(solucion[i]),int(solucion[j])]
            par.sort()
            pares.append(par)
    return pares
{% endhighlight %}

Para encontrar cuales combinaciones de números ya han salido en sorteos anteriores y poder estimar esta probabilidad usamos una lista con el contenido de los últimos sorteos:

{% highlight python %}
def probabilidad_repetir(solucion):
    repetir = 1
    if solucion in historico["cinco_numeros"]:
        repetir = 1/2118760
    return repetir
{% endhighlight %}


Para calcular las matrices de frecuencias, podemos hacer lo siguiente:
{% highlight python %}
def crear_historico(self, sorteos = 104, ruta = "euromillions.csv"):
        archivo = open(ruta, 'r')
        lector_csv = csv.reader(archivo, delimiter=';')
        datos_csv = list(lector_csv)
        del datos_csv[0:2]
        del datos_csv[sorteos:]
        estrellas = []
        pares = []
        numeros = []

        for linea in datos_csv:
            del linea[0]
            estrellas_linea = [linea[5],linea[6]]
            estrellas_linea.sort()
            estrellas.append(estrellas_linea)
            pares = pares + pares_solucion(linea[:5])
            numeros = numeros + [int(x) for x in linea[:5]]

        frecuencias = [0] * 51
        for numero in numeros:
            frecuencias[numero] = frecuencias[numero] + 1

        frecuencias_pares = [0] * 51
        for i in range(51):
            frecuencias_pares[i] = [0] * 51

        for par in pares:
            i = par[0]
            j = par[1]
            frecuencias_pares[i][j] = frecuencias_pares[i][j] + 1


        numeros.sort()
        return {'frecuencias':frecuencias,
                'frecuencias_pares':frecuencias_pares,
                'cinco_numeros': numeros}
{% endhighlight %}

Si calculamos la frecuencia con la que salio cada número en el último año y observamos los 10 números más frecuentes:

| Número | Frecuencia |
|-------|--------|
| 5  | 17 |
| 15 | 17 |
| 11 | 15 |
| 14 | 14 |
| 19 | 14 |
| 20 | 14 |
| 46 | 14 |
| 12 | 13 |
| 16 | 13 |
| 27 | 13 |

 Y el resultado de la matriz `frecuencias_pares` es este:

 <img src="/notas/assets/euromillones/matriz.png" alt="Individuo1" width="100%"/>

 Cabe mencionar, que todos los números de los sorteos estan ordenados de menor a mayor. Por lo que en los pares, el primer número (eje X) siempre será el menor de ambos, por lo que en la matriz, el triangulo inferior está vacio.

 Por último faltaría calcular las probabilidades de las estrellas. Esa parte decidi dejarla fuera de nuestra función objetivo, eso hará que los números de las estrellas sean al azar totalmente. Escoger esos dos digitos de acuerdo a las frecuencias va a hacer que siempre salgan las mismas dos estrellas y es algo que no quiero que suceda en este programa.

## Operadores genéticos
Para garantizar que nuestro algoritmo funcione correctamente es necesario elegir operadores que generen soluciones válidas para nuestro problema.

### Crossover
Para combinar dos padres en dos hijos, pienso que la mejor opción es usar un _Uniform Crossover_ modificado para lograr mantener las soluciones sin elementos repetidos. El _Uniform Crossover_ trata a cada elemento por separado. Por cada gen se lanza una moneda para decidir si ese gen se hereda del primer padre o del segundo padre.

La modificación para evitar repeticiones es heredar directamente los elementos que esten en ambos padres a los hijos. Y al resto de los genes heredarlos usando _Uniform Crossover_. Una forma de implementarlo podría ser la siguiente:

1. Juntar todos los genes de ambos padres en dos conjuntos, uno para las estrellas y el otro para el resto de los elementos.
2. Por cada elemento en el conjunto de los números, elegir un número aleatorio entre 0 y 1.
3. Si ese número aleatorio es menor a .5, asignar gen al hijo 1, excepcion que el hijo ya tenga ese gen, en ese caso se asigna al hijo 2. Si es mayor a .5, sucede exactamente lo contrario.
4. Si alguno de los hijos ya tiene suficientes elementos, asingar el resto de los elementos al otro hijo.
5. Repetir el proceso con el conjunto de estrellas.


{% highlight python %}
def cruzar(padre_1, padre_2):
    numeros = padre_1[0:5] + padre_2[0:5]
    estrellas = padre_1[5:] + padre_2[5:]
    hijo_1 = []
    hijo_2 = []

    for i in numeros:
        if hijo_1.count(i) > 0:
            hijo_2.append(i)
        elif hijo_2.count(i) > 0
            hijo_1.append(i)
        elif len(hijo_1) == 5:
            hijo_2.append(i)
        elif len(hijo_2) == 5:
            hijo_1.append(i)
        elif(random.uniform(0,1) > .5):
            hijo_1.append(i)
        else:
            hijo_2.append(i)

    for i in estrellas:
        if hijo_1[5:].count(i) > 0:
            hijo_2.append(i)
        elif hijo_2[5:].count(i) > 0
            hijo_1.append(i)
        elif len(hijo_1) == 7:
            hijo_2.append(i)
        elif len(hijo_2) == 7:
            hijo_1.append(i)
        elif(random.uniform(0,1) > .5):
            hijo_1.append(i)
        else:
            hijo_2.append(i)

    return hijo_1, hijo_2

{% endhighlight %}


### Mutación
Para añadir un poco de diversidad a nuestros números de EuroMillones es necesario usar la mutación. Para este fin, vamos a cambiar aleatoriamente dos números en las soluciones, una vez más respetando las condiciones para que sigan siendo válidas.

1. Seleccionamos un indice entre 0 y 6 aleatoriamente.
2. Seleccionamos un número aleatorio entre 1 y 50 (1 y 12 para las estrellas).
3. Si el número generado ya se encuentra en la solucion, generamos otro.
4. Reemplazamos el numero en el indicie seleccionado por el número generado.
5. Repetir el proceso una vez más.


{% highlight python %}
def mutar(solucion):
    for _ in range(2):
        indice = random.randint(0, 6)
        if indice < 5:
            numero = random.randint(1, 50)
            while(numero in solucion[0:5]):
                numero = random.randint(1, 50)
            solucion[indice] = numero
        else:
            numero = random.randint(1, 12)
            while(numero in solucion[5:]):
                numero = random.randint(1, 12)
            solucion[indice] = numero

    return solucion
{% endhighlight %}

### Selección
La selección que más me gusta a mi es por torneo, y tambien he visto en varios problemas que es de las que mejor desempeño tienen. La selección por torneo funciona de la siguiente forma:
1. Aleatoriamente se eligen $$N$$ individuos de la población.
2. De esos $$N$$ individuos seleccionados, se eligen 2 padres, los 2 individuos con la aptitud más alta de de los $$N$$ individuos.

En Python:
{% highlight python %}
def seleccion_torneo(poblacion):
        indices_torneo = []
        for i in range(0, N):
            indice_seleccionado = randint(0, len(poblacion))

            indices_torneo.append(indice_seleccionado)

        max_1 = -1000
        max_2 = -1000
        padre_1 = None
        padre_2 = None

        for i in indices_torneo:
            aptitud = poblacion[i].evaluar_solucion()
            if aptitud > max_1:
                max_1 = aptitud
                padre_1 = poblacion[i]
            elif aptitud >= max_2:
                max_2 = aptitud
                padre_2 = poblacion[i]

        return (padre_1, padre_2)
{% endhighlight %}

## Notas finales
Ya que tenemos definidas todas las piezas, es cuestión de acomodarlas para que funcionen juntas. Para facilitar la ejecución y la reutilización del código, separé las partes que son exclusivamente de EuroMillones de las funciones del algoritmo génetico.

Para obtener resultados rápidos y evitar que siempre converjan al mismo número, el tamaño de la población la configuré a 50 y el número de generaciones a 150.

Ejemplo de una búsqueda:

{% highlight bash %}
alejandro@Alejandros-MacBook-Pro euromillions % python3 ejecutar.py
Generation 0: 1.8703587293088776e-23
Generation 1: 4.717230525044238e-14
Generation 2: 3.977655156215683e-12
Generation 3: 4.660934358284451e-12
.
.
.
Generation 148: 1.6914165338285278e-10
Generation 149: 1.6914165338285278e-10
Mejor combinación: [21, 17, 4, 23, 27, 4, 10]
{%endhighlight%}

Es importante recalcar que la finalidad este programa es explicar como resolver problemas con un algoritmo genético. Es imposible predecir el futuro y la lotería es un juego de azar, por lo tanto no existe un método que nos haga ganar. Y aun que esta idea tenga ciertos fundamentos teóricos, los números que genera el programa tiene exactamente la misma probabilidad de ganar que si compramos un boleto con los números `[1,2,3,4,5,6,7]` (o cualquier otra combinación).

Además, los cálculos hechos no reflejan un probabilidad real de ganar la lotería y simplemente creamos una métrica para encontrar combinaciones de números que _se lleven bien juntos_.

## Código
El código fuente lo pueden descargar de github: [Algoritmo EuroMillones](https://github.com/Alter93/genetic_euromillions)

Para ejecutar es necesario usar Python 3, y llamar al archivo `ejecutar.py`.
