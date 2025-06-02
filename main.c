// librerie standard del C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// libreria helper
#include "debug.h"

// define
#define BUFFER_SIZE 50
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
    int dist;
} Change_cost_data;

void queue_push(Queue *queue, const void *data, size_t data_size)
{
    if (queue->head == NULL)
    {
        queue->head = (struct Queue_node *)malloc(sizeof(Queue_node));
        queue->head->next = NULL;
        queue->head->data = malloc(data_size);
        memcpy(queue->head->data, data, data_size);
        queue->back = queue->head;
        queue->size = 1;
    }
    else
    {
        queue->back->next = (struct Queue_node *)malloc(sizeof(Queue_node));
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
    queue->size--;
    free(queue->head->data);
    if (queue->head->next)
    {
        Queue_node *tmp = queue->head->next;
        free(queue->head);
        queue->head = tmp;
    }
    else
    {
        free(queue->head);
        queue->head = NULL;
        queue->back = NULL;
        queue->size = 0;
    }
}

void *queue_front(Queue *queue)
{
    return queue->head->data;
}

int queue_size(Queue *queue)
{
    return queue->size;
}

// Air route definition
typedef struct Air_route
{
    int cost;
    int hexagon_index;
} Air_route;

// Air Route List node
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
        list->head = (struct List_node *)malloc(sizeof(List_node));
        list->back = list->head;
        list->head->next = NULL;
        list->head->previous = NULL;
        list->head->data = malloc(data_size);
        memcpy(list->head->data, data, data_size);
        list->size = 1;
    }
    else
    {
        list->back->next = (struct List_node *)malloc(sizeof(List_node));
        list->back->next->previous = list->back;
        list->back->next->next = NULL;
        list->back = list->back->next;
        if (list->size == 1)
            list->head->next = list->back;

        list->back->data = malloc(data_size);
        memcpy(list->head->data, data, data_size);
        list->size++;
    }
}

void list_pop(List *list, List_node *toRemove)
{
    free(toRemove->data);
    if (toRemove == list->head)
    {
        if (list->head->next)
            list->head = list->head->next;
        list->head->previous = NULL;
        free(toRemove);
        list->size--;

        return;
    }
    if (toRemove == list->back)
    {
        if (list->head->previous)
            list->back = list->back->previous;
        list->back->next = NULL;
        free(toRemove);
        list->size--;

        return;
    }
    List_node *it = list->head;
    while (it != toRemove && it != NULL)
    {
        it = it->next;
    }
    if (it != NULL)
    {
        it->next->previous = it->previous;
        it->previous->next = it->next;
        list->size--;
        free(it);
    }
}

size_t list_size(List *list)
{
    return list->size;
}

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
    {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, -1}, {1, 1}}
};

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
    return x + y * MAPX;
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
    // init variabili grandezza
    MAPSIZE = x * y;
    MAPX = x;
    MAPY = y;
    // rilascio della memoria
    if (map)
        free(map);
    map = (Hexagon *)malloc(MAPSIZE * sizeof(Hexagon));
    // invalidate cache
    // TODO

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
    if (!inBounds(x, y))
        return (STATUS)1;
    if (v < -10 || v > 10)
        return (STATUS)2;
    if (raggio <= 0)
        return (STATUS)3;
    // change cost of the (x,y) hexagon
    int index = toIndex(x, y);
    map[index].cost = map[index].cost + v;

    int *visited = (int *)calloc(MAPSIZE, sizeof(int));
    visited[index] = 1;

    Queue q;
    q.head = NULL;
    q.back = NULL;
    q.size = 0;
    // breeadth first visit
    // TODO
    for (int i = 0; i < 6; i++)
    {
        int adx = x + adiacenze[y % 2][i][0];
        int ady = y + adiacenze[y % 2][i][1];
        if (inBounds(adx, ady))
        {
            Change_cost_data d;
            d.dist = 1;
            d.hexagon_index = toIndex(adx, ady);
            queue_push(&q, &d, sizeof(d));
        }
    }

    while (queue_size(&q))
    {
        Change_cost_data *data = (Change_cost_data *)queue_front(&q);
        int dist = data->dist;
        index = data->hexagon_index;
        queue_pop(&q);
        // process
        if (visited[index] == 0 || visited[index] > dist)
        {
            visited[index] = dist;
            map[index].cost = map[index].cost + v * (raggio - dist) / raggio;
            if (map[index].cost < 0)
                map[index].cost = 1;
            if (dist + 1 < raggio)
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
                        d.dist = dist + 1;
                        d.hexagon_index = toIndex(adx, ady);
                        if (visited[d.hexagon_index] == 0 || visited[d.hexagon_index] > d.dist + 1)
                            queue_push(&q, &d, sizeof(d));
                    }
                }
            }
        }
    }

    free(visited);

    return 0;
}

STATUS toggle_air_route(int x1, int y1, int x2, int y2)
{
    // check if both exagons exist
    if(!inBounds(x1,y1)) return (STATUS)1;
    if(!inBounds(x2,y2)) return (STATUS)2;

    int index = toIndex(x1,y1);
    int target_index = toIndex(x2,y2);

    return (STATUS)0; // OK
}

int travel_cost(int xp, int yp, int xd, int yd);

int main(int argc, char **argv)
{
    // define I/O streams
    FILE *istream = stdin;
    FILE *ostream = stdout;
    // define I/O streams as file
    istream = fopen("input.txt", "r");
    ostream = fopen("output.txt", "w");

    char buffer[BUFFER_SIZE];
    char *input = NULL;
    char *cmd = NULL;
    char *parameters = NULL;
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
        // fprintf(ostream,"%s\n",cmd);
        // DEBUGPRINT(strlen(cmd))
        if (cmd)
            DEBUGPRINT(cmd)
        if (parameters)
            DEBUGPRINT(parameters)

        if (!strncmp(cmd, "init", BUFFER_SIZE))
        {
            // init <colonne> <righe>
            int x, y;
            sscanf(parameters, "%d %d", &x, &y);
            if (!init(x, y))
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
            int x, y, v, raggio;
            sscanf(parameters, "%d %d %d %d", &x, &y, &v, &raggio);
            if (!change_cost(x, y, v, raggio))
                fprintf(ostream, OK);
            else
                fprintf(ostream, KO);
        }

        if (!strncmp(cmd, "toggle_air_route", BUFFER_SIZE))
        {
            // toggle_air_route <x1> <y1> <x2> <y2>
        }

        if (!strncmp(cmd, "travel_cost", BUFFER_SIZE))
        {
            // travel_cost <xp> <yp> <xd> <yd>
        }

    } while (1);

    DEBUGPRINT("Cleaning up...");
    // closing streams
    fclose(istream);
    fclose(ostream);

    // free memory
    if (map)
        free(map);
    DEBUGPRINT("Done");
    return 0;
}