// This is the start of the server code

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

void find_opt_string(int argc, char *argv[], const char *what, char **target)
{
    // make sure we actualy are looking for something
    assert(strlen(what));

    int idx = 1;
    
    while(idx < argc){
        char *it = argv[idx];
        while(*it++ == '-');
        switch (--it - argv[idx]){
            case 2:
                if(strcmp(it, what)) break;
            case 1:
                if(*it == *what){
                    if(idx + 1 < argc){
                        *target = malloc(strlen(argv[idx + 1]) + 1);
                        assert(*target);
                        strcpy(*target, argv[idx++ + 1]);
                    }else{
                        assert(!"missing command line value");
                    }
                }
                break;
        }
        idx++;
    }
}



#include "server.h"


int main(int argc, char *argv[])
{
    // get the port from the command line arguments
    
    char *port = NULL;
    find_opt_string(argc, argv, "port", &port);
    if(port == NULL){ fprintf(stderr, "specify a port\n"); return 0; }
    
    start_listening(port);
    
    // now we run the main thing
    //run_game();
    
    
    return 0;
}
