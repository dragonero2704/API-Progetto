// librerie standard del C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// helper macros
// #define _DEBUG

#ifdef _DEBUG

#define TRACE(x) printf("%s : %llu\n", #x, x)
#define HEXTRACE(x) printf("%s : %x\n", #x, x)
#define STRTRACE(x) printf("%s : %s\n", #x, x)
#define STRINFO(x) printf("%s[%d] : %s\n", #x, x, strlen(x) + 1)
#define DEBUGPRINT(msg) printf("[DEBUG] : %s [END]\n", msg);
#define STAMPPRINT(n, msg) printf("[DEBUG] riga [%d] : %s [END]\n", n, msg)

#else

#define TRACE(x)
#define HEXTRACE(x)
#define STRTRACE(x)
#define STRINFO(x)
#define DEBUGPRINT(msg)
#define STAMPPRINT(n, msg)

#endif

// defines
#define BUFFER_SIZE 50
#define AIR_ROUTE_LIMIT 5
#define CACHE_NOT_FOUND -42
#define OK "OK\n"
#define KO "KO\n"

// type definitions
typedef char STATUS;

// Min heap queue
typedef struct Heap_node
{
    int min_heap_parameter; // 4 bytes
    int hexagon_index;      // 4 bytes
} Heap_node;                // 8 bytes

typedef struct Min_heap
{
    Heap_node *data; // 8 bytes
    size_t size;     // 8 bytes
    size_t capacity; // 8 bytes
} Min_heap;          // 24 bytes

// hashmap
typedef struct Hashmap_node
{
    unsigned int key;          // 8 bytes
    int value;                 // 4 bytes
    struct Hashmap_node *next; // 8 bytes
} Hashmap_node;                // 20 bytes

typedef struct Hashmap
{
    Hashmap_node **map; // 8 bytes
    size_t size;        // 8 bytes
    size_t capacity;    // 8 bytes
} Hashmap;

// Air route definition
typedef struct Air_route
{
    // int cost; Per come è definito il costo della air_route, sarà sempre uguale al costo di terra dell'esagono
    int hexagon_index;      // 4 bytes
    struct Air_route *next; // 8 bytes
} Air_route;                // 12 bytes

// Hexagon type definition
typedef struct Hexagon
{
    // hexagon cost
    int cost; // 4 bytes
    // list of air_routes
    Air_route *air_routes_head; // 8 bytes
} Hexagon;                      // 12 bytes

/* ============== GLOBALI ============== */
Hexagon *map = NULL;
int MAPSIZE = 0;
int MAPX = 0;
int MAPY = 0;
const char adiacenze[2][6][2] = {
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {-1, 1}, {-1, -1}},
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, -1}, {1, 1}}};
Min_heap min_heap_queue;
Hashmap cache;
int *distance_array = NULL;

/* ============ FINE GLOBALI ============ */

// =========================================================== FUNCTIONS =========================================================== //
/**
 * @brief inizializza una air_route
 *
 * @param cost costo di to_init
 * @param hexagon_index esagono di arrivo della air_route
 * @param next puntatore al successivo di to_init
 * @return puntatore a air_route inizializzata
 */
static inline Air_route *air_route_init(int hexagon_index, Air_route *next)
{
    Air_route *to_init = (Air_route *)malloc(sizeof(Air_route));
    to_init->hexagon_index = hexagon_index;
    to_init->next = next;
    return to_init;
}

/**
 * @brief Szudzik pair hashing
 * articolo qui: https://sair.synerise.com/efficient-integer-pairs-hashing/
 *
 * @param x primo numero della coppia
 * @param y secondo numero della coppia
 * @return unsigned int hashed
 */
static inline unsigned int Szudzik(unsigned int x, unsigned int y)
{
    if (x > y)
    {
        // x = max{x,y}
        return x * x + x + y;
    }
    else
    {
        // x  != max{x,y}
        return y * y + x;
    }
}

// hashmap functions
/**
 * @brief svuota l'hashmap
 *
 * @param hashmap puntatore alla hashmap
 */
void hashmap_empty(Hashmap *hashmap)
{
    if (!hashmap->map)
        return;
    if (hashmap->size == 0)
        return;
    for (int i = 0; i < hashmap->capacity; i++)
    {
        // se in quell'indice è stato allocato un Heap_node
        if (hashmap->map[i])
        {
            Hashmap_node *curr = hashmap->map[i];
            Hashmap_node *next = NULL;
            while (curr)
            {
                next = curr->next;
                free(curr);
                curr = next;
            }
        }
        hashmap->map[i] = NULL;
    }
    hashmap->size = 0;
}
/**
 * @brief inizializza la hashmap
 *
 * @param hashmap puntatore a hashmap da inizializzare
 * @param capacity grandezza da allocare
 */
void hashmap_init(Hashmap *hashmap, size_t capacity)
{
    /**
     * capacity dovrebbe essere un numero primo per diminuire conflitti
     * Lista di numeri primi : http://compoasso.free.fr/primelistweb/page/prime/liste_online_en.php
     */
    if (hashmap->map)
    {
        hashmap_empty(hashmap);
        free(hashmap->map);
    }
    hashmap->capacity = capacity;
    hashmap->size = 0;
    hashmap->map = (Hashmap_node **)calloc(hashmap->capacity, sizeof(Hashmap_node *));
}

// hashing con metodo del resto
/**
 * @brief funzione di hash
 *
 * @param hashmap puntatore alla hashmap
 * @param toHash input
 * @return unsigned int
 */
unsigned int hashing_function(Hashmap *hashmap, unsigned int toHash)
{
    unsigned int hashed = toHash % hashmap->capacity;
    return hashed;
}
/**
 * @brief inserisce un elemento nella hashmap
 *
 * @param hashmap puntatore alla hashmap
 * @param key chiave da inserire
 * @param value valore associato alla chiave
 */
void hashmap_insert(Hashmap *hashmap, unsigned int key, int value)
{
    // STAMPPRINT(COMMAND_NUMBER, "INSERTING ELEMENT IN CACHE");
    unsigned int digest = hashing_function(hashmap, key);
    Hashmap_node *new_node = (Hashmap_node *)malloc(sizeof(Hashmap_node));
    new_node->key = key;
    new_node->value = value;
    new_node->next = NULL;
    Hashmap_node *it = hashmap->map[digest];
    if (!it)
    {
        hashmap->map[digest] = new_node;
    }
    else
    {
        while (it->next)
        {
            it = it->next;
        }
        it->next = new_node;
    }
    hashmap->size++;
}
/**
 * @brief elimina una chiave e il suo valore associato
 *
 * @param hashmap puntatore alla hashmap
 * @param key chiave da elimare
 */
void hashmap_delete(Hashmap *hashmap, unsigned int key)
{
    unsigned int digest = hashing_function(hashmap, key);

    Hashmap_node *prev = NULL;
    Hashmap_node *curr = hashmap->map[digest];

    while (curr)
    {
        if (curr->key == key)
        {
            // cancellare chiave: 2 casi
            // curr è la testa
            if (!prev)
            {
                hashmap->map[digest] = curr->next;
            }
            else
            {
                // curr è un nodo nel mezzo
                prev->next = curr->next;
            }
            free(curr);
            break;
        }
        // scorri lista
        prev = curr;
        curr = curr->next;
    }
}
/**
 * @brief cerca una key all'interno della Hashmap hashmap
 *
 * @param hashmap puntatore alla hashmap
 * @param key chiave da cercare
 * @return value se la key è presente nella hashmap, altrimenti -42
 * (userò questa hashmap per inserire distanze quindi i valori possibili sono -1 oppure distanze >= 0)
 */
int hashmap_search(Hashmap *hashmap, unsigned int key)
{
    unsigned int digest = hashing_function(hashmap, key);
    Hashmap_node *list = hashmap->map[digest];
    while (list && list->key != key)
    {
        list = list->next;
    }
    if (list == NULL)
    {
        // STAMPPRINT(COMMAND_NUMBER, "CACHE MISS");
        // chiave non trovata
        return CACHE_NOT_FOUND;
    }
    // STAMPPRINT(COMMAND_NUMBER, "CACHE HIT");
    return list->value;
}
// ============================================================= HEAP ============================================================= //
// heap utilities
static inline int heap_parent(int index)
{
    return (index - 1) / 2;
}

static inline int heap_left(int index)
{
    return (index * 2) + 1;
}

static inline int heap_right(int index)
{
    return (index * 2) + 2;
}

/**
 * @brief raddoppia la capacità dell'heap riallocando min_heap->min_heap_queue in un nuovo array
 *
 * @param min_heap puntatore a min_heap da ingrandire
 */
void heap_grow(Min_heap *min_heap)
{
    // allocazione di memoria per nuovo array, uso calloc per settare tutti i byte a 0
    Heap_node *newarr = (Heap_node *)calloc(min_heap->capacity * 2, sizeof(Heap_node));
    // copia vecchio array nel nuovo array
    memcpy(newarr, min_heap->data, sizeof(Heap_node) * min_heap->capacity);
    free(min_heap->data);
    min_heap->data = newarr;
    min_heap->capacity = min_heap->capacity * 2;
}

// heap functions
/**
 * @brief scambia due elementi nell'heap
 *
 * @param a puntatore Heap_node*
 * @param b puntatore Heap_node*
 */
static inline void heap_swap(Heap_node *a, Heap_node *b)
{
    Heap_node tmp;
    tmp.hexagon_index = a->hexagon_index;
    tmp.min_heap_parameter = a->min_heap_parameter;

    a->hexagon_index = b->hexagon_index;
    a->min_heap_parameter = b->min_heap_parameter;
    b->hexagon_index = tmp.hexagon_index;
    b->min_heap_parameter = tmp.min_heap_parameter;
}

/**
 * @brief inizializza l'heap inizializzando size a 0, capacity e allocando l'array
 *
 * @param min_heap puntatore Min_heap* a heap da inizializzare
 * @param capacity capacità dell'heap
 */
void heap_init(Min_heap *min_heap, size_t capacity)
{
    if (min_heap->data)
        free(min_heap->data);

    min_heap->data = (Heap_node *)calloc(capacity, sizeof(Heap_node));

    min_heap->size = 0;
    // la capacità dell'heap deve essere la potenza di due maggiore più vicina a queue_max_length
    min_heap->capacity = capacity;
}

/**
 * @brief set size a 0
 *
 * @param min_heap puntatore Min_heap* a heap da svuotare
 */
static inline void heap_empty(Min_heap *min_heap)
{
    min_heap->size = 0;
}

/**
 * @brief propaga l'aggiornamento dalle foglie alla radice dell'heap per mantenere
 * la proprietà del min-heap. Questa funzione viene chiamata quando viene aggiunto un nuovo elemento allo heap
 * che trovandosi in fondo all'array deve magari essere scambiato con l'elemento padre per metterlo alla radice
 *
 * @param min_heap puntatore a min_heap
 * @param index indice
 */
void heap_heapify_bottom_up(Min_heap *min_heap, int index)
{
    while (index > 0 && min_heap->data[heap_parent(index)].min_heap_parameter > min_heap->data[index].min_heap_parameter)
    {
        // scambia index e padre di index
        int parent_index = heap_parent(index);

        // scambio dei valori
        heap_swap(&min_heap->data[parent_index], &min_heap->data[index]);

        index = parent_index;
    }
}

/**
 * @brief propaga l'aggiornamento dalla radice alle foglie dell'heap per mantenere
 * la proprietà del min-heap.
 * Questa funzione viene chiamata quando viene eliminato un elemento dallo heap
 *
 * @param min_heap puntatore a min_heap
 * @param index
 */
void heap_heapify_top_down(Min_heap *min_heap, int index)
{
    int min_index = index;
    int left, right;
    do
    {
        left = heap_left(index);
        if (left < min_heap->size && min_heap->data[left].min_heap_parameter < min_heap->data[min_index].min_heap_parameter)
        {
            min_index = left;
        }

        right = heap_right(index);
        if (right < min_heap->size && min_heap->data[right].min_heap_parameter < min_heap->data[min_index].min_heap_parameter)
        {
            min_index = right;
        }

        if (min_index == index)
            return;

        heap_swap(&min_heap->data[index], &min_heap->data[min_index]);
        index = min_index;
    } while (left < min_heap->size && right < min_heap->size);
}
/**
 * @brief inserisce un nuovo elemento nell'heap
 *
 * @param min_heap puntatore a min_heap
 * @param node
 */
void heap_push(Min_heap *min_heap, Heap_node node)
{
    min_heap->data[min_heap->size].hexagon_index = node.hexagon_index;
    min_heap->data[min_heap->size].min_heap_parameter = node.min_heap_parameter;

    min_heap->size++;
    if (min_heap->size == min_heap->capacity)
        heap_grow(min_heap);
    heap_heapify_bottom_up(min_heap, min_heap->size - 1);
}

/**
 * @brief Restituisce la radice del min_heap
 *
 * @param min_heap puntatore a min_heap
 * @return Heap_node
 */
Heap_node heap_front(Min_heap *min_heap)
{
    return min_heap->data[0];
}

/**
 * @brief Rimuove la radice del min_heap
 *
 * @param min_heap
 * @return Heap_node
 */
Heap_node heap_pop(Min_heap *min_heap)
{
    // copia dentro result del primo elemento dell'heap
    Heap_node result;
    result.hexagon_index = min_heap->data[0].hexagon_index;
    result.min_heap_parameter = min_heap->data[0].min_heap_parameter;

    // scambia primo e ultimo elemento all'interno dell'array data
    heap_swap(&min_heap->data[0], &min_heap->data[min_heap->size - 1]);
    min_heap->size--;

    // chiama heapify per ricostruire il min_heap top-bottom
    heap_heapify_top_down(min_heap, 0);

    return result;
}

/**
 * @brief controlla se le coordinate fornite sono all'interno della mappa
 *
 * @param x ascissa
 * @param y ordinata
 * @return 0 on success, !=0 altrimenti
 */
static inline STATUS inBounds(int x, int y)
{
    return (STATUS)(x >= 0 && y >= 0 && x < MAPX && y < MAPY);
}

/**
 * @brief restituisce l'indice all'interno dell'array linearizzato date che le coordinate (x,y)
 *
 * @param x ascissa
 * @param y ordinata
 * @param index corrispondente all'interno dell'array linearizzato
 * @return status 0 = OK, error altrimenti
 */
static inline int toIndex(int x, int y)
{
    return y * MAPX + x;
}

/**
 * @brief a parite da un indice VALIDO, calcola le coordinate (x,y)
 *
 * @param index indice dell'array
 * @param x ascissa
 * @param y ordinata
 */
static inline void toCoord(int index, int *x, int *y)
{
    *x = index % MAPX;
    *y = index / MAPX;
}

/**
 * @brief chiama free() sulla mappa degli esagoni Hexagon *map liberando prima le air_route di un esagono se presenti
 *
 */
static inline void free_map()
{
    for (int i = 0; i < MAPSIZE; i++)
    {
        if (map[i].air_routes_head)
        {
            // se sono presenti air_route, deallocarle
            Air_route *tmp = map[i].air_routes_head;
            Air_route *tmp_next = NULL;
            while (tmp)
            {
                tmp_next = tmp->next;
                free(tmp);
                tmp = tmp_next;
            }
            map[i].air_routes_head = NULL;
        }
    }
    free(map);
}

// init
/**
 * @brief inizializza la mappa dati il numero di colonne x e il numero di righe y
 *
 * @param x numero di colonne
 * @param y numero di righe
 * @return STATUS 0 on success
 */
STATUS init(int x, int y)
{
    if (x <= 0 || y <= 0)
        return (STATUS)1;

    // rilascio della memoria
    if (map)
    {
        free_map();
    }
    // init variabili grandezza
    MAPSIZE = x * y;
    MAPX = x;
    MAPY = y;
    map = (Hexagon *)malloc(MAPSIZE * sizeof(Hexagon));
    // init cycle
    for (int i = 0; i < MAPSIZE; i++)
    {
        map[i].cost = 1; // costo unitario di default
        // map[i].air_routes_active = 0;  // size della lista
        map[i].air_routes_head = NULL; // set head pointer to null
    }
    // init data structures per travel_cost e change cost
    heap_init(&min_heap_queue, MAPSIZE);
    hashmap_empty(&cache);
    if (distance_array)
        free(distance_array);
    distance_array = (int *)malloc(sizeof(int) * MAPSIZE);
    return (STATUS)0; // OK
}

/**
 * @brief funzione di costo calcolata per l'esagono e le rotte aeree
 * \n
 * FONDAMENTALE l'uso del float, senza il quale questa funzione darebbe
 * un risultato differente (e errato)
 * \n
 * float fraction = (float)(raggio - distance) / (float)(raggio) * (float)(v);
 * \n
 * int delta = fraction >= 0 ? (int)fraction : (int)(fraction) - 1;
 * \n
 * int newcost = original_cost + delta;
 * @param original_cost costo originario
 * @param v             fattore da scalare v passato nella change_cost
 * @param raggio        raggio fornito dalla change_cost
 * @param distance      distanza attuale
 * @return newcost
 */
static inline int calculate_new_cost(int original_cost, int v, int raggio, int distance)
{
    // =====================================================
    float fraction = (float)(raggio - distance) / (float)(raggio) * (float)(v);
    int delta = fraction >= 0 ? (int)fraction : (int)(fraction)-1;
    int newcost = original_cost + delta;
    // =====================================================
    if (newcost < 0)
        newcost = 0;
    if (newcost > 100)
        newcost = 100;
    return newcost;
}

/**
 * @brief la funzione change_cost cambia il costo di un esagono (x,y) di un fattore v, propagando il cambiamento agli
 * esagoni adiacenti ignorando costi, intransitabilità (costo = 0) e rotte aeree di v scalato per la distanza relativa al raggio
 *
 * @param x ascissa esagono
 * @param y ordinata esagono
 * @param v fattore di costo
 * @param raggio raggio di propagazione
 * @return STATUS 0 on success !=0 altrimenti
 */
STATUS change_cost(int x, int y, int v, int raggio)
{
    // controlla se esiste un puntatore alla mappa
    if (!map)
        return (STATUS)1;
    if (!inBounds(x, y))
        return (STATUS)2;
    if (v < -10 || v > 10)
        return (STATUS)3;
    if (raggio <= 0)
        return (STATUS)4;

    TRACE(cache.size);
    // invalidate cache
    hashmap_empty(&cache);

    // calcola nuovo costo esagono d'origine
    int origin = toIndex(x, y);
    map[origin].cost = calculate_new_cost(map[origin].cost, v, raggio, 0);

    // se raggio == 1 non c'è ulteriore propagazione
    if (raggio == 1)
        return (STATUS)0;

    for (int i = 0; i < MAPSIZE; i++)
    {
        distance_array[i] = 0x7FFFFFFF; // set a INT_MAX che rappresenta infty
    }
    distance_array[origin] = 0;

    // svuota l'heap
    heap_empty(&min_heap_queue);

    // inserire esagono di partenza all'interno dell'heap
    heap_push(&min_heap_queue, (Heap_node){0, origin});

    while (min_heap_queue.size)
    {
        Heap_node data = heap_pop(&min_heap_queue);
        int index = data.hexagon_index;
        int current_distance = data.min_heap_parameter;
        int curx, cury;
        toCoord(index, &curx, &cury);
        // check se la distanza + 1 è minore del raggio
        if (current_distance + 1 < raggio)
        {
            // aggiungi i figli alla coda
            for (int i = 0; i < 6; i++)
            {
                int adx = curx + adiacenze[cury % 2][i][0];
                int ady = cury + adiacenze[cury % 2][i][1];
                int ad_index = toIndex(adx, ady);
                if (inBounds(adx, ady) && distance_array[ad_index] > current_distance + 1)
                {
                    distance_array[ad_index] = current_distance + 1;
                    map[ad_index].cost = calculate_new_cost(map[ad_index].cost, v, raggio, current_distance + 1);
                    heap_push(&min_heap_queue, (Heap_node){current_distance + 1, ad_index});
                }
            }
        }
    }

    // print_map(x, y, v, raggio);
    return (STATUS)0;
}
/**
 * @brief crea (o cancella se esiste) una air route dall'esagono (x1, y1) all'esagono (x2, h2)
 * Il costo della rotta è calcolato come la media di tutte le rotte esistenti + il costo dell'esagono di partenza
 * (rimane uguale al costo di terra)
 *
 * @param x1 ascissa dell'esagono di partenza
 * @param y1 ordinata dell'esagono di partenza
 * @param x2 ascissa dell'esagono di arrivo
 * @param y2 ordinata dell'esagono di arrivo
 * @return STATUS
 */
STATUS toggle_air_route(int x1, int y1, int x2, int y2)
{
    // controlla se map esiste
    if (!map)
        return (STATUS)1;
    // controlla se entrambi gli esagoni sono validi
    if (!inBounds(x1, y1))
        return (STATUS)2;
    if (!inBounds(x2, y2))
        return (STATUS)3;
    // fine edge cases

    int index = toIndex(x1, y1);
    int target_index = toIndex(x2, y2);

    // invalidate cache
    hashmap_empty(&cache);

    // controlla se sono già presenti air route
    if (map[index].air_routes_head)
    {
        Air_route *found = NULL;
        // prima scorrere le air_route
        Air_route *air_route = map[index].air_routes_head;
        unsigned char air_routes_active = 0;
        while (air_route)
        {
            if (air_route->hexagon_index == target_index)
            {
                found = air_route;
                break;
            }
            air_route = air_route->next;
        }
        if (found == NULL)
        {
            // aggiungo air_route
            if (air_routes_active == 5)
                return (STATUS)4;
            air_route = map[index].air_routes_head;
            while (air_route->next)
            {
                air_route = air_route->next;
            }
            air_route->next = air_route_init(target_index, NULL);
        }
        else
        {
            // elimina route
            if (found == map[index].air_routes_head)
            {
                // cambiare la testa della lista
                map[index].air_routes_head = map[index].air_routes_head->next;
            }
            else
            {
                air_route = map[index].air_routes_head;
                while (air_route->next != found)
                {
                    air_route = air_route->next;
                }
                air_route->next = air_route->next->next;
            }
            free(found);
        }
    }
    else
    {
        // aggiungi prima nuova air route
        map[index].air_routes_head = air_route_init(target_index, NULL);
    }

    return (STATUS)0; // OK
}

/**
 * @brief travel cost calcola il percorso minimo da un esagono (xp,yp) a un esagono (xd, yd)
 *
 * @param xp ascissa dell'esagono di partenza
 * @param yp ordinata dell'esagono di partenza
 * @param xd ascissa dell'esagono di arrivo
 * @param yd ordinata dell'esagono di arrivo
 * @return costo del percorso minimo, -1 se irragiungibile
 */
int travel_cost(int xp, int yp, int xd, int yd)
{
    // edge cases
    if (!map)
        return -1;
    if (!inBounds(xp, yp) || !inBounds(xd, yd))
        return -1;

    int departing = toIndex(xp, yp);
    int arrival = toIndex(xd, yd);
    if (departing == arrival)
        return 0;
    if (map[departing].cost == 0)
        return -1;
    // fine edge cases
    int cached_value = hashmap_search(&cache, Szudzik((unsigned int)departing, (unsigned int)arrival));
    if (cached_value != CACHE_NOT_FOUND)
    {
        return cached_value;
    }
    // valore nella cache non trovato, iniziare esplorazione mappa
    // svuota coda
    heap_empty(&min_heap_queue);
    // set max distance
    for (int i = 0; i < MAPSIZE; i++)
    {
        distance_array[i] = 0x7FFFFFFF;
    }
    distance_array[departing] = 0;
    // inserire esagono d'origine nella coda
    heap_push(&min_heap_queue, (Heap_node){0, departing});

    int current_hexagon_index = 0;
    while (min_heap_queue.size)
    {
        Heap_node current_hexagon = heap_pop(&min_heap_queue);
        current_hexagon_index = current_hexagon.hexagon_index;

        int current_hexagon_x, current_hexagon_y;
        toCoord(current_hexagon_index, &current_hexagon_x, &current_hexagon_y);

        if (current_hexagon_index == arrival)
            break;
        // controllo se non è un nodo intransitabile
        if (map[current_hexagon_index].cost != 0)
        {
            // adiacenze
            for (int i = 0; i < 6; i++)
            {
                int adx = current_hexagon_x + adiacenze[current_hexagon_y % 2][i][0];
                int ady = current_hexagon_y + adiacenze[current_hexagon_y % 2][i][1];
                int ad_index = toIndex(adx, ady);
                // la nuova distanza sarà il costo dell'esagono preso in analisi + la sua distanza dal punto di partenza
                int newdistance = map[current_hexagon_index].cost + distance_array[current_hexagon_index];
                if (inBounds(adx, ady) && distance_array[ad_index] > newdistance)
                {
                    distance_array[ad_index] = newdistance;
                    heap_push(&min_heap_queue, (Heap_node){newdistance, ad_index});
                }
            }

            // air routes
            if (map[current_hexagon_index].air_routes_head)
            {
                // itera le rotte aeree
                Air_route *air_route = map[current_hexagon_index].air_routes_head;
                while (air_route)
                {
                    int air_route_index_destination = air_route->hexagon_index;
                    // il costo sarà dato dal costo della rotta aerea + la distanza del nodo di partenza dalla sorgente
                    int newdistance = map[current_hexagon_index].cost + distance_array[current_hexagon_index];
                    if (newdistance < distance_array[air_route_index_destination])
                    {
                        // la distanza nuova proposta è minore di quella attuale
                        distance_array[air_route_index_destination] = newdistance;
                        heap_push(&min_heap_queue, (Heap_node){newdistance, air_route_index_destination});
                    }
                    air_route = air_route->next;
                }
            }
        }
    }
    int result = distance_array[arrival];
    if (result == 0x7FFFFFFF)
        result = -1;
    hashmap_insert(&cache, Szudzik((unsigned int)departing, (unsigned int)arrival), result);
    return result;
}

int main(int argc, char **argv)
{
    // define I/O streams
    FILE *istream = stdin;
    FILE *ostream = stdout;

    // define I/O streams from file
    // istream = fopen("./test/empty.txt", "r");
    // ostream = fopen("output.txt", "w");
    char buffer[BUFFER_SIZE];
    char *input = NULL;
    char *cmd = NULL;
    char *parameters = NULL;
    int p1, p2, p3, p4;

    // init cache
    hashmap_init(&cache, 101);

    // input loop
    do
    {
        input = (char *)fgets(buffer, BUFFER_SIZE, istream);
        if (!input)
            break;

        // split string in cmd e parameters
        cmd = strtok(input, " ");
        parameters = strtok(NULL, "\0");

        if (!strncmp(cmd, "init", BUFFER_SIZE))
        {
            // init <colonne> <righe>
            sscanf(parameters, "%d %d", &p1, &p2);
            if (!init(p1, p2))
            {
                fprintf(ostream, OK);
            }
            else
            {
                fprintf(ostream, KO);
            }
        }

        if (!strncmp(cmd, "change_cost", BUFFER_SIZE))
        {
            // change_cost <x> <y> <v> <raggio>
            sscanf(parameters, "%d %d %d %d", &p1, &p2, &p3, &p4);
            if (!change_cost(p1, p2, p3, p4))
                fprintf(ostream, OK);
            else
                fprintf(ostream, KO);
        }

        if (!strncmp(cmd, "toggle_air_route", BUFFER_SIZE))
        {
            // toggle_air_route <x1> <y1> <x2> <y2>
            sscanf(parameters, "%d %d %d %d", &p1, &p2, &p3, &p4);
            if (!toggle_air_route(p1, p2, p3, p4))
                fprintf(ostream, OK);
            else
                fprintf(ostream, KO);
        }

        if (!strncmp(cmd, "travel_cost", BUFFER_SIZE))
        {
            // travel_cost <xp> <yp> <xd> <yd>
            sscanf(parameters, "%d %d %d %d", &p1, &p2, &p3, &p4);
            fprintf(ostream, "%d\n", travel_cost(p1, p2, p3, p4));
        }

    } while (input);

    // closing streams
    if (istream != stdin)
        fclose(istream);
    if (ostream != stdout)
        fclose(ostream);

    // free memory
    if (map)
        free_map();
    heap_empty(&min_heap_queue);
    if (min_heap_queue.data)
        free(min_heap_queue.data);
    hashmap_empty(&cache);
    if (cache.map)
        free(cache.map);
    if (distance_array)
        free(distance_array);

    return 0;
}