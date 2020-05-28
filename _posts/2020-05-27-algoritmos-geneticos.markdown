---
layout: post
title:  "Algoritmos genéticos"
date:   2020-05-27 11:17:35 -0500
author:   "Alejandro Alvarez"
categories: tesis
---

Los algoritmos genéticos son una meta-heuristica de búsqueda inspirada en la selección natural.

A partir de una conjunto de soluciones a un problema, el algoritmo selecciona las mejores soluciones y las combina, para así, ir mejorando continuamente es conjunto, hasta encontrar una que tenga las características que buscamos.


## Representación de la solución

Una de las principales características de este algoritmo es la manera en la que se representa la solución a un problema. En la versión original del algoritmo, se propone, que se represente la solución como una cadena de bits, sin embargo, realmente puede ser una cadena de cualquier tipo de dato (números enteros, números reales, letras). Esta cadena siempre debe de ser del mismo tamaño y cada posición define una caracteristica o un comportamiento. Por ejemplo:

_Individuo A:_

<img src="/notas/assets/algoritmo-genetico/individuo1.png" alt="Individuo1" width="75%"/>

_Individuo B:_

<img src="/notas/assets/algoritmo-genetico/individuo2.png" alt="Individuo2" width="75%"/>

## Función objetivo

Esta función es la que va a guiar el algoritmo hacia encontrar mejores soluciones. Normalmente el objetivo de un algoritmo genético es maximizar o minimizar el valor obtenido por una función:

$$ G(x) = \max f(x) $$

ó

$$ G(x) = \min f(x) $$

A esa función $$ f(x) $$ se le conoce como funcion de aptitud. Si quisieramos crear un algoritmo genético para encontrar la representacion binaria del numero 15 en 4 bits, una función de aptitud podria ser:

$$ f(x) = \sum_{i=1}^{4} x_i \cdot 2^i $$

Por lo tanto si tenemos dos individuos:

```
A: [0][1][0][1]
B: [1][0][0][1]
```

La aptitud del individuo A $$f(x_A)$$ es 10 y del individuo B $$f(x_B)$$ es 9, como estamos tratando de maximizar $$f(x)$$, A es mejor solución que B.


## Algoritmo

Para iniciar necesitamos definir los siguientes parámetros:

- $$ n $$: Tamaño de la población.
- $$ f(x) $$: Una función de aptitud.
- Un objetivo: _minimizar_ o _maximizar_.
- Un __criterio de fin de búsqueda__.

El algoritmo funciona de la siguiente manera:

1. Inicialización:<br>
1.1. Se crea una población  $$\mathbb{P}:\{ x_1 ... x_n\} $$<br>
1.2. Se evalua la calidad de cada solución con la función de aptitud.<br>
2. Búsqueda:<br>
2.1. Se seleccionan 2 individuos de nuestra población ($$ x_i,x_j \in \mathbb{P}$$)<br>
2.2. Se aplica el operador de cruza a los dos individuos, generando ($$\hat{x_i},\hat{x_j}$$)<br>
2.3. Se aplica el operador de mutación sobre $$\hat{x_i}$$ y $$\hat{x_j}$$<br>
2.4. Evaluar aptitud $$f(\hat{x_i})$$ y $$f(\hat{x_j})$$<br>
2.5. Agregar $$\hat{x_i}$$ y $$\hat{x_j}$$ a conjunto de soluciones nuevas $$\hat{\mathbb{P}}$$<br>
2.6. Repetir pasos desde 2.1 hasta 2.6 hasta que $$ \mid \hat{\mathbb{P}} \mid = \mid \mathbb{P} \mid $$<br>
2.7. Reemplazar $$\mathbb{P}$$ con $$\hat{\mathbb{P}} $$<br>
2.8. Repetir _búsqueda_ hasta que se cumpla el __criterio de fin de búsqueda__.


## Selección

Existen muchas formas para seleccionar los dos individuos que vamos a cruzar (paso 2.1). Los más populares son:
- Selección por ruleta
- Selección por torneo
- Selección por rango

### Ruleta
En este método de selección, la probabilidad de un individuo de ser seleccionado es proporcional a su aptitud respecto a la población actual. Por lo tanto una solución $$x$$ con aptitud $$f_x$$, tiene una probabilidad de ser seleccionada

$$
p_x = \frac{f_x}{\sum_{i=1}^{n}f_i}
$$

donde $$n$$ es el tamaño de la población.

Si tenemos una población conformada por 4 soluciones $$[x_1, x_2, x_3, x_4]$$ con aptitudes
$$ f_{1} = 3, f_{2} = 4, f_{3} = 8, f_{4} = 1 $$
entonces las probabilidades de cada solución de ser seleccionada serán:

$$
p_{1} = \frac{3}{3+4+8+1} = 0.19
$$

$$
p_{2} = \frac{4}{3+4+8+1} = 0.25
$$

$$
p_{3} = \frac{8}{3+4+8+1} = 0.5
$$

$$
p_{4} = \frac{1}{3+4+8+1} = 0.06
$$

### Rango
Es parecido a la selección por ruleta, pero la probabilidad de un individuo de ser seleccionado es de acuerdo a su posición en la población cuando es ordenada por aptitud. Si una solución $$x$$ tiene posición $$r_x$$ en la población de tamaño $$n$$, la probabilidad de ser seleccionada es de:

$$
p_x = \frac{n - r_x + 1}{\sum_{i=0}^{n}r_i}
$$

Por lo tanto, usando el ejemplo de la selección por ruleta, las probabilidades calculadas usando rangos serían:

$$
p_{1} = \frac{4 - 3 + 1}{1 + 2 + 3 + 4} = 0.2
$$

$$
p_{2} = \frac{4 - 2 + 1}{1 + 2 + 3 + 4} = 0.3
$$

$$
p_{3} = \frac{4 - 1 + 1}{1 + 2 + 3 + 4} = 0.4
$$

$$
p_{4} = \frac{4 - 4 + 1}{1 + 2 + 3 + 4} = 0.1
$$


### Torneo

El método de selección por torneo es un poco más complicado que los otros dos. Consiste en escoger $$k$$ individuos de la población y de esos $$k$$ individuos, seleccionar al más apto. Al reducir el tamaño del torneo ($$k$$) podemos darle más posibilidad a individuos menos aptos de reproducirse. En el torneo se pueden manipular las probabilidades con las que un individuo gana el torneo. Esas probabilidades se pueden calcular con la ruleta o la selección por rango. Normalmente todos los individuos tienen la misma probabilidad de entrar al torneo y el individuo más apto es el que gana el torneo.

1. Seleccionar $$k$$ individuos de la población.
2. Seleccionar al mejor individuo del torneo usando una probabilidad.
3. Seleccionar al segundo mejor individuo del torneo usando una probabilidad.<br>
.<br>
.<br>
.


## Cruza

Una vez seleccionados dos individuos de la población estos dos los combinamos para crear dos nuevas soluciones. La idea es que si la primera y la segunda solución son de buena calidad, los resultados de la cruza deben de ser de buena calidad. Existen muchos métodos para cruzar soluciones, el método que seleccionemos depende principalmente de la estructura de las soluciones. Hay problemas donde es necesario que ningun elemento de las soluciones este repetido, o soluciones donde no se pueden realizar operaciones aritméticas los elementos. La *cruza de un solo punto* y la *cruza de k puntos*, funcionan para soluciones binarias y también númericas.

La *cruza de orden 1* sirve para problemas donde las soluciones siempre contienen exactamente los mismos elementos y simplemente se altera el orden de los elementos.

Existen muchos más operadores de cruza, estos son solo algunos muy usados.

### Cruza de un solo punto
Si tenemos dos soluciones A y B, de tamaño $$N$$:
1. Se selecciona un número aleatorio $$k \mid 0 < k < N $$
2. Los elementos del 0 al $$k$$ del padre A se copian al hijo A.
3. Los elementos del $$k$$ al $$N$$ del padre B se copian al hijo A.
4. Los elementos del 0 al $$k$$ del padre B se copian al hijo B.
5. Los elementos del $$k$$ al $$N$$ del padre A se copian al hijo B.

<img src="/notas/assets/algoritmo-genetico/singlepoint.png" alt="cruza" width="75%"/>

### Cruza de k puntos
Si tenemos dos soluciones A y B, de tamaño $$N$$:
1. Se seleccionan $$k$$ números $$ C = \{c \mid 0 \le c < N\}$$
2. Para cada número $$ c \in C $$
3. Los elementos del $$k$$ al $$N$$ del padre B se copian al hijo A.
4. Los elementos del 0 al $$k$$ del padre B se copian al hijo B.
5. Los elementos del $$k$$ al $$N$$ del padre A se copian al hijo B.


<img src="/notas/assets/algoritmo-genetico/multipunto.png" alt="cruza" width="72%"/>

### Cruza orden 1

## Mutación



## Variantes

## Aplicaciones
