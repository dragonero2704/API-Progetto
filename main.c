#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// libreria helper
#include "debug.h"

// define
#define BUFFER_SIZE 50
#define OK "OK\n"
#define KO "KO\n"

// type definitions
// Air route definition
typedef struct 
{
    int cost;
    int hexagon_index;
} Air_route;

// Air Route List node
typedef struct
{
    Air_route air_route;
    struct Air_route_node* previous;
    struct Air_route_node* next;
} Air_route_node;

typedef Air_route_node* Air_route_list;

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
    Air_route_list air_routes;
    int air_routes_number;
} Hexagon;

// global vars
Hexagon* map = NULL;
int MAPSIZE = 0;
int MAPX = 0;
int MAPY = 0;

// Functions
int toIndex(int x, int y)
{
    return x + y*MAPX;
}
int outOfBounds(int x, int y)
{
    int index = toIndex(x,y);
    return index < 0 || index >= MAPSIZE;
}
// init
int init(int x, int y)
{
    if(x < 0 || y < 0) return -1;
    // init variabili grandezza
    MAPSIZE = x*y;
    MAPX = x;
    MAPY = y;
    // rilascio della memoria
    if(map) free(map);
    map = (Hexagon*)malloc(MAPSIZE*sizeof(Hexagon));
    
    // init cycle
    for(int i = 0; i < MAPSIZE; i++)
    {
        map[i].cost         = 1;            // costo unitario di default
        map[i].x            = i%MAPX;       // l'ascissa è data dalla colonna della mappa
        map[i].y            = i/MAPX;       // l'ordinata è data dalla riga della mappa
        map[i].air_routes   = NULL;         // puntatore a null [ALLOCARE MEMORIA]
        map[i].air_routes_number = 0;       // size della lista
    }
    return 0; // OK
}

int main(int argc, char** argv)
{
    char buffer[BUFFER_SIZE];
    // define I/O streams
    FILE* instream = stdin;
    FILE* outstream = stdout;

    // define I/O streams as file
    instream = fopen("input.txt", "r");
    outstream = fopen("output.txt", "a");
    char* input = NULL;
    char* cmd = NULL;
    char *parameters = NULL;
    // input loop
    do
    {
        input = fgets(buffer, BUFFER_SIZE, instream);
        // delete trailing "\n"
        input[strlen(input)-1] = '\0';
        // split string in command and parameters
        cmd = strtok(input, " ");
        parameters = strtok(NULL, "\0");
        if(cmd == NULL) break;
        //fprintf(outstream,"%s\n",cmd);
        //DEBUGPRINT(strlen(cmd))
        if(cmd) DEBUGPRINT(cmd)
        if(parameters) DEBUGPRINT(parameters)

        if(!strncmp(cmd, "init", BUFFER_SIZE))
        {
            // init <colonne> <righe>
            int x,y;
            sscanf(parameters, "%d %d", &x, &y);
            if(!init(x,y))
            {
                fprintf(outstream, OK);
            }
            else
            {
                fprintf(outstream, KO);
            }

        }

        if(!strncmp(cmd, "change_cost", BUFFER_SIZE))
        {
            // change_cost <x> <y> <v> <raggio>
            int x,y,v,raggio;
            sscanf(parameters, "%d %d %d %d", &x, &y, &v, &raggio);
        }

        if(!strncmp(cmd, "toggle_air_route", BUFFER_SIZE))
        {
            // toggle_air_route <x1> <y1> <x2> <y2>

        }

        if(!strncmp(cmd, "travel_cost", BUFFER_SIZE))
        {
            // travel_cost <xp> <yp> <xd> <yd>

        }

    } while (1);
    
    DEBUGPRINT("Cleaning up...");
    // closing streams
    fclose(instream);
    fclose(outstream);

    // free memory
    if(map) free(map);
    DEBUGPRINT("Done");
    return 0;
}