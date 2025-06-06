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
#define BUFFER_SIZE 50
#define AIR_ROUTE_LIMIT 5
#define CACHE_UNSET -42
#define OK "OK\n"
#define KO "KO\n"

// type definitions
typedef char STATUS;

// Min heap queue
typedef struct Queue_node
{
    int min_heap_parameter;
    int hexagon_index;
} Queue_node;

typedef struct Heap_queue
{
    Queue_node *queue;
    size_t size;
    size_t capacity;
} Heap_queue;

typedef struct Change_cost_data
{
    int hexagon_index;
    int distance;
} Change_cost_data;

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

// global vars
Hexagon *map = NULL;
int MAPSIZE = 0;
int MAPX = 0;
int MAPY = 0;
int adiacenze[2][6][2] = {
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {-1, 1}, {-1, -1}},
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, -1}, {1, 1}}};
// int *cache = NULL;
Heap_queue travel_cost_queue;
Heap_queue change_cost_queue;

// Functions
int toIndex(int x, int y);
void print_map()
{
    printf("MAPY : %d\nMAPX : %d\nMAPSIZE : %d\n", MAPY, MAPX, MAPSIZE);
    //char **matrix[MAPY][MAPX];
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
    Air_route *tmp = a;
    *a = *b;
    *b = *tmp;
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

void heap_swap(Queue_node *a, Queue_node *b)
{
    Queue_node tmp;
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
void heap_init(Heap_queue *q, size_t queue_max_lenght)
{
    if (q->queue)
        free(q->queue);
    q->queue = (Queue_node *)calloc(queue_max_lenght, sizeof(Queue_node));
    q->size = 0;
    q->capacity = queue_max_lenght;
}

/**
 * @brief setta size a 0
 *
 * @param q
 */
void heap_empty(Heap_queue *q)
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
void heap_heapify_bottom_up(Heap_queue *q, int index)
{
    while (index > 0 && q->queue[heap_parent(index)].min_heap_parameter > q->queue[index].min_heap_parameter)
    {
        // scambia index e padre di index
        heap_swap(&q->queue[heap_parent(index)], &q->queue[index]);
        index = heap_parent(index);
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
void heap_heapify_top_bottom(Heap_queue *q, int index)
{
    int min_index = index;
    int left = heap_left(index);
    if (left < q->size && q->queue[left].min_heap_parameter < q->queue[min_index].min_heap_parameter)
    {
        min_index = left;
    }

    int right = heap_right(index);
    if (right < q->size && q->queue[right].min_heap_parameter < q->queue[min_index].min_heap_parameter)
    {
        min_index = right;
    }

    if (min_index != index)
    {
        heap_swap(&q->queue[index], &q->queue[min_index]);
        // propaga aggiornamento heap verso il basso (foglie)
        heap_heapify_top_bottom(q, min_index);
    }
}

void heap_push(Heap_queue *q, Queue_node node)
{
    q->queue[q->size].hexagon_index = node.hexagon_index;
    q->queue[q->size].min_heap_parameter = node.min_heap_parameter;
    q->size++;

    heap_heapify_bottom_up(q, q->size - 1);
}

Queue_node heap_front(Heap_queue *q)
{
    return q->queue[0];
}

Queue_node heap_pop(Heap_queue *q)
{
    Queue_node result;
    result.hexagon_index = q->queue[0].hexagon_index;
    result.min_heap_parameter = q->queue[0].min_heap_parameter;
    // not working
    heap_swap(&q->queue[0], &q->queue[q->size - 1]);

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
    heap_init(&change_cost_queue, MAPSIZE);
    heap_init(&travel_cost_queue, MAPSIZE);

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
    heap_empty(&change_cost_queue);
    Queue_node heap_data;
    heap_data.hexagon_index = origin;
    heap_data.min_heap_parameter = 0;
    heap_push(&change_cost_queue, heap_data);

    while (change_cost_queue.size)
    {
        Queue_node data = heap_pop(&change_cost_queue);
        int index = data.hexagon_index;
        int current_distance = data.min_heap_parameter;
        // check se la distanza 
        if (current_distance + 1 < raggio)
        {
            // aggiungi i figli alla coda
            for (int i = 0; i < 6; i++)
            {
                int curx, cury;
                toCoord(index, &curx, &cury);
                int adx = curx + adiacenze[cury % 2][i][0];
                int ady = cury + adiacenze[cury % 2][i][1];
                int ad_index = toIndex(adx, ady);
                if (ad_index == origin)
                    continue;
                if (inBounds(adx, ady) && distance_array[ad_index] > current_distance + 1)
                {
                    distance_array[ad_index] = current_distance + 1;
                    int newcost = map[ad_index].cost +
                                  v * (raggio - current_distance - 1) / raggio;
                    if (newcost < 0)
                        newcost = 0;
                    if (newcost > 100)
                        newcost = 100;
                    map[ad_index].cost = newcost;
                    // processa rotte aeree dell'adiacenza
                    for (int i = 0; i < map[ad_index].air_routes_active; i++)
                    {
                        newcost = map[ad_index].air_routes[i].cost +
                                  v * (raggio - current_distance - 1) / raggio;
                        if (newcost < 0)
                            newcost = 0;
                        if (newcost > 100)
                            newcost = 100;
                        map[ad_index].air_routes[i].cost = newcost;
                    }
                    Queue_node d;
                    d.min_heap_parameter = current_distance + 1;
                    d.hexagon_index = ad_index;
                    heap_push(&change_cost_queue, d);
                }
            }
        }
    }
    heap_empty(&change_cost_queue);
    free(distance_array);
    //print_map();
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
    if (map[index].air_routes_active >= AIR_ROUTE_LIMIT)
        return (STATUS)4;
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

// TODO BUGFIX
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

    // svuota coda
    heap_empty(&travel_cost_queue);
    int *distance = (int *)malloc(MAPSIZE * sizeof(int));
    // set max distance
    for (int i = 0; i < MAPSIZE; i++)
    {
        distance[i] = 0x7FFFFFFF;
    }
    distance[departing] = 0;
    Queue_node heap_data;
    heap_data.hexagon_index = departing;
    heap_data.min_heap_parameter = 0;
    heap_push(&travel_cost_queue, heap_data);

    // l'errore è qui
    int current_hexagon_index = 0;
    while (travel_cost_queue.size)
    {
        Queue_node current_hexagon = heap_pop(&travel_cost_queue);
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
                    Queue_node qn;
                    qn.min_heap_parameter = newdistance;
                    qn.hexagon_index = ad_index;
                    heap_push(&travel_cost_queue, qn);
                }
            }

            // air routes
            if(map[current_hexagon_index].air_routes_active)
            {
                // itera le rotte aeree
                for(int i = 0; i < map[current_hexagon_index].air_routes_active; i++)
                {
                    int air_route_index = map[current_hexagon_index].air_routes[i].hexagon_index;
                    int air_route_cost = map[current_hexagon_index].air_routes[i].cost;
                    // il costo sarà dato dal costo della rotta aerea + la distanza del nodo di partenza dalla sorgente
                    int newdistance = air_route_cost + distance[current_hexagon_index];
                    if(newdistance < distance[air_route_index])
                    {
                        // la distanza nuova proposta è minore di quella attuale
                        distance[air_route_index] = newdistance;
                        Queue_node qn;
                        qn.min_heap_parameter = newdistance;
                        qn.hexagon_index = air_route_index;
                        heap_push(&travel_cost_queue, qn);
                    }
                }
            }
            
        }
    }

    int result = distance[arrival];
    free(distance);
    distance = NULL;
    heap_empty(&travel_cost_queue);
    if (result == 0x7FFFFFFF)
        return -1;
    return result;
}

int main(int argc, char **argv)
{
    // define I/O streams
    FILE *istream = stdin;
    FILE *ostream = stdout;
    // define I/O streams as file
    //istream = fopen("./test/large.txt", "r");
    //ostream = fopen("output.txt", "w");

    char buffer[BUFFER_SIZE];
    char *input = NULL;
    char *cmd = NULL;
    char *parameters = NULL;
    int p1, p2, p3, p4;
    // input loop
    do
    {
        do
        {
            input = (char *)fgets(buffer, BUFFER_SIZE, istream);
        } while (!input);

        // delete trailing "\n"
        buffer[strlen(input) - 1] = '\0';
        // split string in command and parameters
        cmd = strtok(buffer, " ");
        parameters = strtok(NULL, "\0");
        if (cmd == NULL)
            break;

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

    } while (1);

    DEBUGPRINT("Cleaning up...");
    // closing streams
    fclose(istream);
    fclose(ostream);

    // free memory
    if (map)
    {
        free(map);
    }

    return 0;
}