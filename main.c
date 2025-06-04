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

// queue
typedef struct Queue_node
{
    void *data;
    struct Queue_node *next;
} Queue_node;

typedef struct Queue
{
    struct Queue_node *head;
    struct Queue_node *back;
    size_t size;
} Queue;

typedef struct Change_cost_data
{
    int hexagon_index;
    int distance;
} Change_cost_data;

void queue_push(Queue *queue, const void *data, size_t data_size)
{
    if (queue->head == NULL)
    {
        queue->head = (Queue_node *)malloc(sizeof(Queue_node));
        queue->head->next = NULL;
        queue->head->data = malloc(data_size);
        memcpy(queue->head->data, data, data_size);
        queue->back = queue->head;
        queue->size = 1;
    }
    else
    {
        queue->back->next = (Queue_node *)malloc(sizeof(Queue_node));
        queue->back = queue->back->next;
        queue->back->data = malloc(data_size);
        queue->back->next = NULL;
        memcpy(queue->back->data, data, data_size);
        if (queue->size == 1)
            queue->head->next = queue->back;
        queue->size += 1;
    }
}

void queue_pop(Queue *queue)
{
    if (!queue || !queue->head)
        return;
    Queue_node *toRemove = queue->head;
    queue->head = queue->head->next;
    if (!queue->head)
        queue->back = NULL;

    if (toRemove->data)
        free(toRemove->data);
    free(toRemove);
    queue->size--;
}

void *queue_front(Queue *queue)
{
    return queue->head->data;
}

int queue_size(Queue *queue)
{
    return queue->size;
}

void queue_destroy(Queue *queue)
{
    while (queue->head)
    {
        queue_pop(queue);
    }
}

// Air route definition
typedef struct Air_route
{
    int cost;
    int hexagon_index;
} Air_route;

// List node
typedef struct List_node
{
    void *data;
    struct List_node *previous;
    struct List_node *next;
} List_node;

typedef struct List
{
    struct List_node *head;
    struct List_node *back;
    size_t size;
} List, *PList;

void list_push(List *list, void *data, size_t data_size)
{
    if (list->head == NULL)
    {
        // empty list
        list->head = (List_node *)malloc(sizeof(List_node));
        list->head->next = NULL;
        list->head->previous = NULL;
        list->back = list->head;
        // data allocation
        list->head->data = malloc(data_size);
        memcpy(list->head->data, data, data_size);
        list->size = 1;
    }
    else
    {
        list->back->next = (List_node *)malloc(sizeof(List_node));
        list->back->next->previous = list->back;
        list->back = list->back->next;
        list->back->next = NULL;
        list->back->data = malloc(data_size);
        memcpy(list->back->data, data, data_size);
        list->size++;
    }
}

void list_pop(List *list, List_node *toRemove)
{
    if (!list || !toRemove || list->size == 0)
        return;

    if (list->size == 1 && list->head == toRemove)
    {
        free(toRemove->data);
        free(toRemove);
        list->head = NULL;
        list->back = NULL;
        list->size = 0;
        return;
    }

    if (toRemove == list->head)
    {
        list->head = list->head->next;
        if (list->head)
            list->head->previous = NULL;
        else
            list->back = NULL;
        free(toRemove->data);
        free(toRemove);
        list->size--;
        return;
    }

    if (toRemove == list->back)
    {
        list->back = list->back->previous;
        if (list->back)
            list->back->next = NULL;
        else
            list->head = NULL;
        free(toRemove->data);
        free(toRemove);
        list->size--;
        return;
    }

    toRemove->previous->next = toRemove->next;
    toRemove->next->previous = toRemove->previous;

    free(toRemove->data);
    free(toRemove);
    list->size--;
    return;
}

void list_destroy(List *list)
{
    while (list->head)
        list_pop(list, list->head);
}

size_t list_size(List *list)
{
    return list->size;
}
// Stack

// Hexagon type definition
typedef struct
{
    // hexagon cost
    int cost;
    // hexagon coordinates
    int x; // colonna
    int y; // riga
    // id = x + y*N_COLONNE
    // air routes
    List air_routes;
} Hexagon;

// global vars
Hexagon *map = NULL;
int MAPSIZE = 0;
int MAPX = 0;
int MAPY = 0;
int adiacenze[2][6][2] = {
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {-1, 1}, {-1, -1}},
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, -1}, {1, 1}}};
int *cache = NULL;

// Functions
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
        // destroy air_route_list
        for (int i = 0; i < MAPSIZE; i++)
        {
            list_destroy(&map[i].air_routes);
        }
        free(map);
    }
    // init variabili grandezza
    MAPSIZE = x * y;
    MAPX = x;
    MAPY = y;
    map = (Hexagon *)malloc(MAPSIZE * sizeof(Hexagon));
    // invalidate cache
    // if (cache)
    //     free(cache);
    // cache = (int *)malloc(2 * MAPSIZE * sizeof(int));
    // for (int i = 0; i < 2 * MAPSIZE; i++)
    //     cache[i] = CACHE_UNSET;

    // init cycle
    for (int i = 0; i < MAPSIZE; i++)
    {
        map[i].cost = 1;               // costo unitario di default
        map[i].x = i % MAPX;           // l'ascissa è data dalla colonna della mappa
        map[i].y = i / MAPX;           // l'ordinata è data dalla riga della mappa
        map[i].air_routes.head = NULL; // puntatore a null [ALLOCARE MEMORIA]
        map[i].air_routes.back = NULL; // puntatore a null [ALLOCARE MEMORIA]
        map[i].air_routes.size = 0;    // size della lista
    }
    return (STATUS)0; // OK
}

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
    int index = toIndex(x, y);
    map[index].cost = map[index].cost + v;
    if (map[index].cost < 0)
        map[index].cost = 0;
    if (map[index].cost > 100)
        map[index].cost = 100;
    int *visited = (int *)calloc(MAPSIZE, sizeof(int));
    visited[index] = 1;

    Queue q;
    q.head = NULL;
    q.back = NULL;
    q.size = 0;
    // breeadth first visit
    for (int i = 0; i < 6; i++)
    {
        int adx = x + adiacenze[y % 2][i][0];
        int ady = y + adiacenze[y % 2][i][1];
        if (inBounds(adx, ady))
        {
            Change_cost_data d;
            d.distance = 1;
            d.hexagon_index = toIndex(adx, ady);
            queue_push(&q, &d, sizeof(d));
        }
    }

    while (queue_size(&q))
    {
        Change_cost_data *data = (Change_cost_data *)queue_front(&q);
        int distance = data->distance;
        index = data->hexagon_index;
        queue_pop(&q);
        // process
        if (visited[index] == 0 || visited[index] > distance)
        {
            visited[index] = distance;
            map[index].cost = map[index].cost + v * (raggio - distance) / raggio;
            // limit the cost to the interval [1,100]
            if (map[index].cost < 0)
                map[index].cost = 0;
            if (map[index].cost > 100)
                map[index].cost = 100;
            if (map[index].air_routes.size != 0)
            {
                List_node *it = map[index].air_routes.head;
                while (it)
                {
                    Air_route *a = (Air_route *)(it->data);
                    a->cost = a->cost + v * (raggio - distance) / raggio;
                    if (a->cost < 0)
                        a->cost = 0;
                    if (a->cost > 100)
                        a->cost = 100;
                    it = it->next;
                }
            }

            if (distance + 1 < raggio)
            {
                // aggiungi i figli alla coda
                for (int i = 0; i < 6; i++)
                {
                    int curx = map[index].x;
                    int cury = map[index].y;
                    int adx = curx + adiacenze[cury % 2][i][0];
                    int ady = cury + adiacenze[cury % 2][i][1];
                    if (inBounds(adx, ady))
                    {
                        Change_cost_data d;
                        d.distance = distance + 1;
                        d.hexagon_index = toIndex(adx, ady);
                        if (visited[d.hexagon_index] == 0 || visited[d.hexagon_index] > d.distance)
                            queue_push(&q, &d, sizeof(d));
                    }
                }
            }
        }
    }
    queue_destroy(&q);
    free(visited);

    return 0;
}
/**
 * @brief creates (or deletes if it already exist) an air route starting from (x1, y1) hexagon and ending in (x2, h2) hexagon
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

    // loop through starting hexagon air_routes_list
    List_node *iterator = map[index].air_routes.head;
    List_node *air_route_present = NULL;
    int average = map[index].cost;
    while (iterator)
    {
        Air_route air_route = *(Air_route *)iterator->data;
        average += air_route.cost;
        if (air_route.hexagon_index == target_index)
        {
            air_route_present = iterator;
            break;
        }
        iterator = iterator->next;
    }

    if (air_route_present)
    {
        // remove the air_route
        list_pop(&map[index].air_routes, air_route_present);
    }
    else
    {
        // add a new route
        if (map[index].air_routes.size >= AIR_ROUTE_LIMIT)
        {
            // can't add a new route
            return (STATUS)4;
        }
        Air_route new_route;
        new_route.hexagon_index = target_index;
        new_route.cost = average / (1 + map[index].air_routes.size);
        if (new_route.cost < 0)
            new_route.cost = 0;
        if (new_route.cost > 100)
            new_route.cost = 100;
        list_push(&map[index].air_routes, (void *)&new_route, sizeof(Air_route));
    }

    return (STATUS)0; // OK
}

typedef struct Hexagon_coords
{
    int x;
    int y;
} Hexagon_coords;

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

    Queue q;
    q.head = NULL;
    q.back = NULL;
    q.size = 0;

    int *distance = (int *)calloc(MAPSIZE, sizeof(int));
    // set max distance
    for (int i = 0; i < MAPSIZE; i++)
    {
        distance[i] = 0x7FFFFFFF;
    }
    distance[departing] = 0;

    // adiacenze

    for (int i = 0; i < 6; i++)
    {
        int adx = xp + adiacenze[yp % 2][i][0];
        int ady = yp + adiacenze[yp % 2][i][1];
        int index = toIndex(adx, ady);
        if (inBounds(adx, ady) && distance[index] > map[departing].cost + distance[departing])
        {
            distance[index] = map[departing].cost;
            Hexagon_coords hc;
            hc.x = adx;
            hc.y = ady;
            queue_push(&q, &hc, sizeof(hc));
        }
    }

    // air routes
    if (map[departing].air_routes.size)
    {
        List_node *it = map[departing].air_routes.head;
        while (it)
        {
            Air_route *air_route = (Air_route *)it->data;
            Hexagon_coords hc;
            toCoord(air_route->hexagon_index, &hc.x, &hc.y);
            if (distance[air_route->hexagon_index] > air_route->cost + distance[departing])
            {
                distance[air_route->hexagon_index] = air_route->cost + distance[departing];
                queue_push(&q, &hc, sizeof(Hexagon_coords));
            }
            it = it->next;
        }
    }
    Hexagon_coords current_hexagon_coordinates;
    int current_hexagon_index = 0;
    while (q.size)
    {
        current_hexagon_coordinates = *(Hexagon_coords *)queue_front(&q);
        current_hexagon_index = toIndex(current_hexagon_coordinates.x, current_hexagon_coordinates.y);
        queue_pop(&q);
        if (map[current_hexagon_index].cost != 0)
        {
            // adiacent
            for (int i = 0; i < 6; i++)
            {
                int adx = current_hexagon_coordinates.x + adiacenze[current_hexagon_coordinates.y % 2][i][0];
                int ady = current_hexagon_coordinates.y + adiacenze[current_hexagon_coordinates.y % 2][i][1];
                int index = toIndex(adx, ady);
                if (inBounds(adx, ady) && distance[index] > map[current_hexagon_index].cost + distance[current_hexagon_index])
                {
                    distance[index] = map[current_hexagon_index].cost + distance[current_hexagon_index];
                    Hexagon_coords hc;
                    hc.x = adx;
                    hc.y = ady;
                    queue_push(&q, &hc, sizeof(hc));
                }
            }

            // air routes
            if (map[current_hexagon_index].air_routes.size)
            {
                List_node *it = map[current_hexagon_index].air_routes.head;
                while (it)
                {
                    Air_route *air_route = (Air_route *)it->data;
                    Hexagon_coords hc;
                    toCoord(air_route->hexagon_index, &hc.x, &hc.y);
                    if (distance[air_route->hexagon_index] > air_route->cost + distance[current_hexagon_index])
                    {
                        distance[air_route->hexagon_index] = air_route->cost + distance[current_hexagon_index];
                        queue_push(&q, &hc, sizeof(hc));
                    }
                    it = it->next;
                }
            }
        }
    }

    int result = distance[arrival];
    free(distance);
    distance = NULL;
    queue_destroy(&q);
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
    istream = fopen("./test/large.txt", "r");
    ostream = fopen("output.txt", "w");

    char buffer[BUFFER_SIZE];
    char *input = NULL;
    char *cmd = NULL;
    char *parameters = NULL;
    int p1, p2, p3, p4;
    // input loop
    do
    {
        input = fgets(buffer, BUFFER_SIZE, istream);
        // delete trailing "\n"
        input[strlen(input) - 1] = '\0';
        // split string in command and parameters
        cmd = strtok(input, " ");
        parameters = strtok(NULL, "\0");
        if (cmd == NULL)
            break;
        if (cmd)
            DEBUGPRINT(cmd)
        if (parameters)
            DEBUGPRINT(parameters)

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
        // destroy air_route_list
        for (int i = 0; i < MAPSIZE; i++)
        {
            list_destroy(&map[i].air_routes);
        }
        free(map);
    }
    DEBUGPRINT("Done");
    return 0;
}