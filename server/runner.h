#ifndef RUNNER_H_
#define RUNNER_H_

#include <windows.h>

struct player;

// listens for commands from the client and updates the serverside
// state to reflect the clients actions
void run_client(SOCKET client_sock, struct player *player);











#endif//RUNNER_H_
