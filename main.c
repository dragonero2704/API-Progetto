// librerie standard del C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// helper macros
// #define _DEBUG

#ifdef _DEBUG
#include <stdio.h>

#define TRACE(x) printf("%s : %d", #x, x)
#define HEXTRACE(x) printf("%s : %x", #x, x)
#define STRTRACE(x) printf("%s : %s", #x, x)
#define STRINFO(x) printf("%s[%d] : %s", #x, x, strlen(x) + 1)
#define DEBUGPRINT(msg) printf("[DEBUG] : %s [END]\n", msg);

#else

#define TRACE(x)
#define HEXTRACE(x)
#define STRTRACE(x)
#define STRINFO(x)
#define DEBUGPRINT(msg)

#endif

// defines
#define BUFFER_SIZE 100
#define AIR_ROUTE_LIMIT 5
#define CACHE_NOT_FOUND -42
#define OK "OK\n"
#define KO "KO\n"

// type definitions
typedef char STATUS;

// Min heap queue
typedef struct Heap_node
{
    int min_heap_parameter;
    int hexagon_index;
} Heap_node;

typedef struct Min_heap
{
    Heap_node *min_heap;
    size_t size;
    size_t capacity;
} Min_heap;

// hashmap
typedef struct Hashmap_node
{
    /*
     * la chiave sarà la concatenazione degli indici degli esagono strutturati in questo modo:
     * partenza.arrivo = partenza * 10 ^ n + arrivo
     */
    size_t key;
    int value;
    struct Hashmap_node *next;
} Hashmap_node;

typedef struct Hashmap
{
    Hashmap_node **map;
    size_t size;
    size_t capacity;
} Hashmap;

// Air route definition
typedef struct Air_route
{
    int cost;
    int hexagon_index;
} Air_route;

// Hexagon type definition
typedef struct
{
    // hexagon cost
    int cost;
    Air_route air_routes[5];
    int air_routes_active;
} Hexagon;

/* ============== GLOBALI ============== */
Hexagon *map = NULL;
int MAPSIZE = 0;
int MAPX = 0;
int MAPY = 0;
int adiacenze[2][6][2] = {
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {-1, 1}, {-1, -1}},
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, -1}, {1, 1}}};
// int *cache = NULL;
Min_heap min_heap_queue;
Hashmap cache;
/* ============ FINE GLOBALI ============ */

// Functions
int toIndex(int x, int y);
void print_map()
{
    printf("MAPY : %d\nMAPX : %d\nMAPSIZE : %d\n", MAPY, MAPX, MAPSIZE);
    // char **matrix[MAPY][MAPX];
    for (int i = 0; i < MAPY; i++)
    {
        for (int j = 0; j < MAPX; j++)
        {
            if (i % 2 == 0)
            {
                printf("%d ", map[toIndex(j, i)].cost);
            }
            else
            {
                if (j == 0)
                    printf(" ");
                printf("%d ", map[toIndex(j, i)].cost);
            }
        }
        printf("\n");
    }
    printf("\nEND OF PRINT\n");
}

void air_route_swap(Air_route *a, Air_route *b)
{
    Air_route tmp;
    tmp.cost = a->cost;
    tmp.hexagon_index = a->hexagon_index;
    // swap
    a->cost = b->cost;
    a->hexagon_index = b->hexagon_index;

    b->cost = tmp.cost;
    b->hexagon_index = tmp.hexagon_index;
    return;
}
// Szudzik pair hashing
// articolo qui: https://sair.synerise.com/efficient-integer-pairs-hashing/
size_t Szudzik(unsigned int x, unsigned int y)
{
    if (x > y)
    {
        // x = max{x,y}
        return (size_t)x * x + x + y;
    }
    else
    {
        // x  != max{x,y}
        return (size_t)y * y + x;
    }
}

// hashmap functions
void hashmap_empty(Hashmap *h)
{
    if (!h->map)
        return;
    if (h->size == 0)
        return;
    for (int i = 0; i < h->capacity; i++)
    {
        // se in quell'indice è stato allocato un Heap_node
        if (h->map[i])
        {
            Hashmap_node *curr = h->map[i];
            Hashmap_node *next = NULL;
            while (curr)
            {
                next = curr->next;
                free(curr);
                curr = next;
            }
        }
        h->map[i] = NULL;
    }
    h->size = 0;
}
/**
 * @brief
 *
 * @param h puntatore alla hashmap da inizializzare
 */
void hashmap_init(Hashmap *h)
{
    /**
     * Un numero primo per diminuire conflitti
     * Lista di numeri primi : http://compoasso.free.fr/primelistweb/page/prime/liste_online_en.php
     */
    if (h->map)
    {
        hashmap_empty(h);
        free(h->map);
    }
    h->capacity = 577;
    h->size = 0;
    h->map = (Hashmap_node **)calloc(h->capacity, sizeof(Hashmap_node *));
}

// hashing con metodo della divisione
size_t hashing_function(Hashmap *h, size_t toHash)
{
    size_t hashed = toHash % h->capacity;
    return hashed;
}

void hashmap_insert(Hashmap *h, size_t key, int value)
{
    DEBUGPRINT("INSERTING ELEMENT IN CACHE")
    size_t digest = hashing_function(h, key);
    Hashmap_node *new_node = (Hashmap_node *)malloc(sizeof(Hashmap_node));
    new_node->key = key;
    new_node->value = value;
    new_node->next = NULL;
    Hashmap_node *it = h->map[digest];
    if (!it)
    {
        h->map[digest] = new_node;
    }
    else
    {
        while (it->next)
        {
            it = it->next;
        }
        it->next = new_node;
    }
    h->size++;
}

void hashmap_delete(Hashmap *h, size_t key)
{
    size_t digest = hashing_function(h, key);

    Hashmap_node *prev = NULL;
    Hashmap_node *curr = h->map[digest];

    while (curr)
    {
        if (curr->key == key)
        {
            // cancellare chiave: 2 casi

            // curr è la testa
            if (!prev)
            {
                h->map[digest] = curr->next;
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
 * @brief cerca una key all'interno della Hashmap h
 *
 * @param h puntatore alla hashmap
 * @param key chiave da cercare
 * @return value se la key è presente nella hashmap, altrimenti -42
 * (userò questa hashmap per inserire distanze quindi i valori possibili sono -1 oppure distanze >= 0)
 */
int hashmap_search(Hashmap *h, size_t key)
{
    size_t digest = hashing_function(h, key);
    Hashmap_node *list = h->map[digest];
    while (list && list->key != key)
    {
        list = list->next;
    }
    if (list == NULL)
    {
        DEBUGPRINT("CACHE MISS");
        // chiave non trovata
        return CACHE_NOT_FOUND;
    }
    DEBUGPRINT("CACHE HIT");
    return list->value;
}

// heap utilities
int heap_parent(int index)
{
    return (index - 1) / 2;
}

int heap_left(int index)
{
    return (index * 2) + 1;
}

int heap_right(int index)
{
    return (index * 2) + 2;
}

// heap functions
void heap_swap(Heap_node *a, Heap_node *b)
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
 * @param q
 * @param queue_max_lenght
 */
void heap_init(Min_heap *q, size_t queue_max_lenght)
{
    if (q->min_heap)
        free(q->min_heap);

    q->min_heap = (Heap_node *)calloc(queue_max_lenght, sizeof(Heap_node));

    q->size = 0;
    q->capacity = queue_max_lenght;
}

/**
 * @brief setta size a 0
 *
 * @param q
 */
void heap_empty(Min_heap *q)
{
    q->size = 0;
}

/**
 * @brief propaga l'aggiornamento dalle foglie alla radice dell'heap per mantenere
 * la proprietà del min-heap. Questa funzione viene chiamata quando viene aggiunto un nuovo elemento allo heap
 * che trovandosi in fondo all'array deve magari essere scambiato con l'elemento padre per metterlo alla radice
 *
 * @param q
 * @param index
 */
void heap_heapify_bottom_up(Min_heap *q, int index)
{
    while (index > 0 && q->min_heap[heap_parent(index)].min_heap_parameter > q->min_heap[index].min_heap_parameter)
    {
        // scambia index e padre di index
        int parent_index = heap_parent(index);

        // scambio dei valori
        heap_swap(&q->min_heap[parent_index], &q->min_heap[index]);

        index = parent_index;
    }
}

/**
 * @brief propaga l'aggiornamento dalla radice alle foglie dell'heap per mantenere
 * la proprietà del min-heap.
 * Questa funzione viene chiamata quando viene eliminato un elemento dallo heap
 *
 * @param q
 * @param index
 */
void heap_heapify_top_bottom(Min_heap *q, int index)
{
    int min_index = index;
    int left = heap_left(index);
    if (left < q->size && q->min_heap[left].min_heap_parameter < q->min_heap[min_index].min_heap_parameter)
    {
        min_index = left;
    }

    int right = heap_right(index);
    if (right < q->size && q->min_heap[right].min_heap_parameter < q->min_heap[min_index].min_heap_parameter)
    {
        min_index = right;
    }

    if (min_index != index)
    {
        heap_swap(&q->min_heap[index], &q->min_heap[min_index]);
        // propaga aggiornamento heap verso il basso (foglie)
        heap_heapify_top_bottom(q, min_index);
    }
}

void heap_push(Min_heap *q, Heap_node node)
{
    q->min_heap[q->size].hexagon_index = node.hexagon_index;
    q->min_heap[q->size].min_heap_parameter = node.min_heap_parameter;

    q->size++;
    heap_heapify_bottom_up(q, q->size - 1);
}

Heap_node heap_front(Min_heap *q)
{
    return q->min_heap[0];
}

Heap_node heap_pop(Min_heap *q)
{
    Heap_node result;
    result.hexagon_index = q->min_heap[0].hexagon_index;
    result.min_heap_parameter = q->min_heap[0].min_heap_parameter;
    heap_swap(&q->min_heap[0], &q->min_heap[q->size - 1]);

    q->size--;

    heap_heapify_top_bottom(q, 0);

    return result;
}

/**
 * @brief checks if the given x, y coordinates are inBounds
 *
 * @param x
 * @param y
 * @return int
 */
STATUS inBounds(int x, int y)
{
    return (STATUS)(x >= 0 && y >= 0 && x < MAPX && y < MAPY);
}

/**
 * @brief returns a status of the conversion of x y coordinates in
 * an index array, if the x,y coordiantes are out of bounds, it returns -1, 0 otherwise
 *
 * @param x x coordinate
 * @param y y coordinate
 * @param index the index corrisponding to the array calculated via y * MAPX + x
 * @return status 0 = OK, error otherwise
 */
int toIndex(int x, int y)
{
    return y * MAPX + x;
}

void toCoord(int index, int *x, int *y)
{
    *x = index % MAPX;
    *y = index / MAPX;
}

// init
/**
 * @brief initialize map given the number of columns x and the number of rows y
 *
 * @param x number of columns
 * @param y number of rows
 * @return STATUS 0 on success
 */
STATUS init(int x, int y)
{
    if (x <= 0 || y <= 0)
        return (STATUS)1;

    // rilascio della memoria
    if (map)
    {
        free(map);
    }
    // init variabili grandezza
    MAPSIZE = x * y;
    MAPX = x;
    MAPY = y;
    map = (Hexagon *)malloc(MAPSIZE * sizeof(Hexagon));
    // init cycle
    for (int i = 0; i < MAPSIZE; i++)
    {
        map[i].cost = 1;              // costo unitario di default
        map[i].air_routes_active = 0; // size della lista
    }
    // init data structures per travel_cost e change cost
    heap_init(&min_heap_queue, MAPSIZE);
    hashmap_empty(&cache);
    return (STATUS)0; // OK
}
// TODO CORRECT: NOT PROPAGATING
/**
 * @brief
 *
 * @param x
 * @param y
 * @param v
 * @param raggio
 * @return STATUS
 */
STATUS change_cost(int x, int y, int v, int raggio)
{
    // check if map exists
    if (!map)
        return (STATUS)1;
    if (!inBounds(x, y))
        return (STATUS)2;
    if (v < -10 || v > 10)
        return (STATUS)3;
    if (raggio <= 0)
        return (STATUS)4;
    // invalidate cache
    hashmap_empty(&cache);
    // change cost of the (x,y) hexagon
    int origin = toIndex(x, y);
    map[origin].cost = map[origin].cost + v;
    if (map[origin].cost < 0)
        map[origin].cost = 0;
    if (map[origin].cost > 100)
        map[origin].cost = 100;

    // rotte aeree dell'origine
    for (int i = 0; i < map[origin].air_routes_active; i++)
    {
        int newcost = map[origin].air_routes[i].cost + v;
        if (newcost < 0)
            newcost = 0;
        if (newcost > 100)
            newcost = 100;
        map[origin].air_routes[i].cost = newcost;
    }

    if (raggio == 1)
        return (STATUS)0;

    int *distance_array = (int *)malloc(MAPSIZE * sizeof(int));
    for (int i = 0; i < MAPSIZE; i++)
    {
        distance_array[i] = 0x7FFFFFFF;
    }
    distance_array[origin] = 0;
    // TODO BUGFIX

    // svuota l'heap
    heap_empty(&min_heap_queue);
    Heap_node heap_data;
    heap_data.hexagon_index = origin;
    heap_data.min_heap_parameter = 0;
    heap_push(&min_heap_queue, heap_data);

    while (min_heap_queue.size)
    {
        Heap_node data = heap_pop(&min_heap_queue);
        int index = data.hexagon_index;
        int current_distance = data.min_heap_parameter;
        int curx, cury;
        toCoord(index, &curx, &cury);
        // check se la distanza
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
                    Heap_node d;
                    d.min_heap_parameter = current_distance + 1;
                    d.hexagon_index = ad_index;
                    heap_push(&min_heap_queue, d);
                }
            }
        }
    }
    // spostare parte in cui si aggiornano i costi qui
    // iterare attraverso tutte le distanze, e se la distanza è != 0x7FFFFFFF
    // aggiornare il costo del nodo e delle sue rotte aeree
    int newcost = 0;
    for (int i = 0; i < MAPSIZE; i++)
    {
        if (distance_array[i] != 0x7FFFFFFF)
        {
            // aggiornare esagono i
            newcost = map[i].cost + v * (raggio - distance_array[i]) / raggio;
            // limitare a [0,100]
            if (newcost < 0)
                newcost = 0;
            if (newcost > 100)
                newcost = 100;
            map[i].cost = newcost;
            // aggiornare rotte aeree
            for (int j = 0; j < map[i].air_routes_active; j++)
            {
                newcost =
                    map[i].air_routes[j].cost + v * (raggio - distance_array[i]) / raggio;
                if (newcost < 0)
                    newcost = 0;
                if (newcost > 100)
                    newcost = 100;
                map[i].air_routes[j].cost = newcost;
            }
        }
    }
    free(distance_array);
    return (STATUS)0;
}
/**
 * @brief crea (o cancella se esiste) una air route dall'esagono (x1, y1) allìesagono (x2, h2)
 * Il costo della rotta è calcolato come la media di tutte le rotte esistenti + il costo dell'esagono di partenza
 *
 * @param x1 ascissa dell'esagono di partenza
 * @param y1 ordinata dell'esagono di partenza
 * @param x2 ascissa dell'esagono di arrivo
 * @param y2 ordinata dell'esagono di arrivo
 * @return STATUS
 */
STATUS toggle_air_route(int x1, int y1, int x2, int y2)
{
    // check if map exists
    if (!map)
        return (STATUS)1;
    // check if both exagons exist
    if (!inBounds(x1, y1))
        return (STATUS)2;
    if (!inBounds(x2, y2))
        return (STATUS)3;

    int index = toIndex(x1, y1);
    int target_index = toIndex(x2, y2);

    // invalidate cache
    hashmap_empty(&cache);

    int air_routes_active = map[index].air_routes_active;
    if (air_routes_active)
    {
        int found = -1;
        int average = map[index].cost;
        // prima scorrere le air_route
        for (int i = 0; i < air_routes_active; i++)
        {
            average += map[index].air_routes[i].cost;
            int hexagon_index = map[index].air_routes[i].hexagon_index;
            if (hexagon_index == target_index)
            {
                found = i;
                break;
            }
        }
        if (found == -1)
        {
            // aggiungo air_route
            if(map[index].air_routes_active == 5) return (STATUS)4;
            map[index].air_routes[air_routes_active].cost = average / (1 + air_routes_active);
            map[index].air_routes[air_routes_active].hexagon_index = target_index;
            map[index].air_routes_active++;
        }
        else
        {
            // elimina route
            if (found != air_routes_active)
                air_route_swap(&map[index].air_routes[found], &map[index].air_routes[air_routes_active - 1]);
            map[index].air_routes_active--;
        }
    }
    else
    {
        // aggiungi prima nuova air route
        map[index].air_routes[air_routes_active].hexagon_index = target_index;
        map[index].air_routes[air_routes_active].cost = map[index].cost;
        map[index].air_routes_active = 1;
    }

    return (STATUS)0; // OK
}

/**
 * @brief travel cost calcola il percorso minimo
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
    int *distance = (int *)malloc(MAPSIZE * sizeof(int));
    // set max distance
    for (int i = 0; i < MAPSIZE; i++)
    {
        distance[i] = 0x7FFFFFFF;
    }
    distance[departing] = 0;
    Heap_node heap_data;
    heap_data.hexagon_index = departing;
    heap_data.min_heap_parameter = 0;
    heap_push(&min_heap_queue, heap_data);

    // l'errore è qui
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
                int newdistance = map[current_hexagon_index].cost + distance[current_hexagon_index];
                if (inBounds(adx, ady) && distance[ad_index] > newdistance)
                {
                    distance[ad_index] = newdistance;
                    Heap_node qn;
                    qn.min_heap_parameter = newdistance;
                    qn.hexagon_index = ad_index;
                    heap_push(&min_heap_queue, qn);
                }
            }

            // air routes
            if (map[current_hexagon_index].air_routes_active)
            {
                // itera le rotte aeree
                for (int i = 0; i < map[current_hexagon_index].air_routes_active; i++)
                {
                    int air_route_index_destination = map[current_hexagon_index].air_routes[i].hexagon_index;
                    int air_route_cost = map[current_hexagon_index].air_routes[i].cost;
                    // il costo sarà dato dal costo della rotta aerea + la distanza del nodo di partenza dalla sorgente
                    int newdistance = air_route_cost + distance[current_hexagon_index];
                    if (newdistance < distance[air_route_index_destination])
                    {
                        // la distanza nuova proposta è minore di quella attuale
                        distance[air_route_index_destination] = newdistance;
                        Heap_node qn;
                        qn.min_heap_parameter = newdistance;
                        qn.hexagon_index = air_route_index_destination;
                        heap_push(&min_heap_queue, qn);
                    }
                }
            }
        }
    }
    // RESULT:
    int result = distance[arrival];

    free(distance);
    distance = NULL;
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
    // define I/O streams as file
    istream = fopen("./testlong.txt", "r");
    ostream = fopen("output.txt", "w");

    char buffer[BUFFER_SIZE];
    char *input = NULL;
    char *cmd = NULL;
    char *parameters = NULL;
    int command_found = 0;
    int p1, p2, p3, p4;
    // init cache
    hashmap_init(&cache);
    // input loop
    unsigned int commands_read = 0;
    do
    {
        command_found = 0;
        do
        {
            input = (char *)fgets(buffer, BUFFER_SIZE, istream);
        } while (!input);
        commands_read += 1;
        // split string in command and parameters
        cmd = strtok(input, " ");
        parameters = strtok(NULL, "\0");

        if (!strncmp(cmd, "init", BUFFER_SIZE))
        {
            command_found = 1;
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
            command_found = 1;
            // change_cost <x> <y> <v> <raggio>
            sscanf(parameters, "%d %d %d %d", &p1, &p2, &p3, &p4);
            if (!change_cost(p1, p2, p3, p4))
                fprintf(ostream, OK);
            else
                fprintf(ostream, KO);
        }

        if (!strncmp(cmd, "toggle_air_route", BUFFER_SIZE))
        {
            command_found = 1;
            // toggle_air_route <x1> <y1> <x2> <y2>
            sscanf(parameters, "%d %d %d %d", &p1, &p2, &p3, &p4);
            if (!toggle_air_route(p1, p2, p3, p4))
                fprintf(ostream, OK);
            else
                fprintf(ostream, KO);
        }

        if (!strncmp(cmd, "travel_cost", BUFFER_SIZE))
        {
            command_found = 1;
            // travel_cost <xp> <yp> <xd> <yd>
            sscanf(parameters, "%d %d %d %d", &p1, &p2, &p3, &p4);
            fprintf(ostream, "%d\n", travel_cost(p1, p2, p3, p4));
        }

    } while (command_found);

    DEBUGPRINT("Cleaning up...");
    // closing streams
    if (istream != stdin)
        fclose(istream);
    if (ostream != stdout)
        fclose(ostream);

    // free memory
    if (map)
    {
        free(map);
    }
    heap_empty(&min_heap_queue);
    if (min_heap_queue.min_heap)
    {
        free(min_heap_queue.min_heap);
    }
    hashmap_empty(&cache);
    if (cache.map)
    {
        free(cache.map);
    }

    return 0;
}