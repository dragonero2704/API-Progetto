# Prova finale API 2024-2025

[![Tests](https://github.com/dragonero2704/API-Progetto/actions/workflows/build.yml/badge.svg)](https://github.com/dragonero2704/API-Progetto/actions/workflows/build.yml)

Progetto finale del corso Algoritmi e Principi di Informatica (API) del Politecnico di Milano A.A. 2024-2025

## Descrizione

La [specifica](specifiche.pdf) del progetto richiede di progettare e scrivere un programma che presi in input da `stdin` i comandi di

- `init <n° colonne> <n° righe>`
- `change_cost <x> <y> <raggio>`
- `toggle_air_route <x1> <y1> <x2> <y2>`
- `travel_cost <xp> <yp> <xd> <yd>`

e relativi parametri, modifichi una mappa formata da esagoni, cambiandone i costi di attraversamento e trovando il percorso migliore, cioè a costo **minimo**, che colleghi un esagono $A$ a un esagono $B$.
Al fine di ottimizzare il programma, nella [specifica](specifiche.pdf) viene dichiarato che la funzione di travel_cost viene chiamata molto più spesso delle altre.

## Implementazione

La mappa ha $x$ "colonne" e $y$ "righe", ciò rende possibile l'utilizzo di un sistema di coordinate cartesiane per identificare univocamente gli esagoni che compongono la mappa. Inoltre si nota che conviene rappresentare la mappa (che è un grafo) sotto forma di matrice di adiacenza, in quanto gli esagoni sono sempre connessi agli esagoni direttamente adiacenti. Inoltre gli esagoni hanno disposizione al massimo $5$ "rotte aeree", cioè $5$ collegamenti a esagoni non adiacenti, rappresentati come archi orientati dall'esagono di partenza a quello di arrivo.

Da queste premesse una possibile struttura dati che descrive un esagono della mappa (nodo del grafo) è la seguente:

```C
typedef struct Air_route
{
    int cost; // costo della rotta aerea
    int hexagon_index; // indice dell'esagono ottenuto tramite linearizzazione della matrice di adiacenza
    struct Air_route *next; // puntatore alla rotta aerea successiva
} Air_route;

typedef struct Hexagon
{
    int cost; // costo di attraversamento
    Air_route *air_routes_head; // puntatore a HEAD della lista di rotte aeree
} Hexagon;
```

Dato che è possibile usare un sistema di coordinate cartesiano per descrivere la mappa degli esagoni. è possibile descrivere le adiacenze (escludendo le rotte aeree) come vettore bidimensioale, differenza tra le coordinate cartesiane di partenza e di arrivo. Ad esempio, l'adiacenza $`\begin{pmatrix}0 \\\ 1\end{pmatrix}`$ rappresenta l'esagono che si trova una riga sopra l'esagono considerato (dalla [specifica](specifiche.pdf) l'origine del piano cartesiano è posizionato in basso a sinistra rispetto alla mappa).
Tutti i nodi hanno 6 adiacenze (essendo esagoni), $4$ di queste sono comuni a tutti i nodi cioè

```math 
\{ \begin{pmatrix}0 \\\ 1\end{pmatrix}, \begin{pmatrix}1 \\\ 0\end{pmatrix}, \begin{pmatrix}0 \\\ -1\end{pmatrix}, \begin{pmatrix}-1 \\\ 0\end{pmatrix}\}
```

Le ultime $2$ adiacenze cambiano in funzione della posizione dell'esagono:

- riga con ordinata **pari**: $`\set{\begin{pmatrix}-1 \\\ 1\end{pmatrix},\begin{pmatrix}-1 \\\ -1\end{pmatrix}}`$
- riga con ordinata **dispari**: $`\set{ \begin{pmatrix}1 \\\ -1\end{pmatrix}, \begin{pmatrix}1 \\\ 1\end{pmatrix} }`$

Un esempio di implementazione in `C` delle adiacenze è un array multidimensionale:

```C
const char adiacenze[2][6][2] = {
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {-1, 1}, {-1, -1}},  // adiacenze pari
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}}     // adiacenze dispari
    };
```

(P.S.) è anche possibile utilizzare un solo vettore di adiacenze e moltiplicare per un fattore di $-1$ quando si esamina un esagono su riga dispari, risparmiando $12$ Byte.

---
Per trovare il percorso a costo minimo tra due esagoni possiamo utilizzare l'algoritmo di Dijistrika, in quanto la [specifica](specifiche.pdf) esclude costi negativi. L'algoritmo necessita di una *priority queue* per ordinare i vertici da visitare in funzione della distanza dal nodo di partenza. Questo comportamento è ottenibile tramite un [min_heap](src/main.c#L40).

Per la funzione `change_cost` è possibile continuare ad applicare l'algoritmo di Dijistrika defeninendo la distanza di un nodo $n$ come numero di esagoni da attraversare per arrivare dal nodo di partenza al nodo $n$.

La funzione `toggle_air_route` richiede di aggiungere (o rimuovere se già presente) una rotta aerea dall'esagono $`\begin{pmatrix}x_p\\ y_p\end{pmatrix}`$ all'esagono $`\begin{pmatrix}x_d\\y_d\end{pmatrix}`$. La logica è molto simile a quella di una funzione che inserisce (o rimuove) un elemento da una *single-linked list*.

Allo scopo di ottimizzare il programma viene suggerito l'uso di una cache per memorizzare i risultati delle chiamate a `travel_cost`, cache implementata tramite [hashmap](src/main.c#L54).
