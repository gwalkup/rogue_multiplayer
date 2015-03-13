// This is where the socket and thread code goes

#include "server.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

//#include "extern.h"

#include "runner.h"

int ready()
{
    return 0;
}

struct player;

struct socket_passer
{
    SOCKET client_sock;
    struct player *player;
};

DWORD WINAPI client_worker(struct socket_passer *env)
{
    run_client(env->client_sock, player);
    free(env);
}

// this runs while the players connect
DWORD WINAPI connecter(SOCKET *listen_sock)
{
    while(TRUE){
        SOCKET client_sock = accept(*listen_sock, NULL, NULL);
        if(client_sock == INVALID_SOCKET){
            fprintf(stderr, "accept error: %ld\n", WSAGetLastError());
            closesocket(*listen_sock);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        
        struct socket_passer *passer = malloc(sizeof(struct socket_passer));
        assert(passer);
        
        passer->client_sock = client_sock;
        //passer->player = add_player();
        
        HANDLE client_worker_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&client_worker, passer, 0, NULL);    
    }
}


void start_listening(const char *port)
{
    WSADATA wsa_data;
    int ret = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if(ret){
        fprintf(stderr, "winsock error: %ld\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    
    struct addrinfo hints = {}, *result;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    ret = getaddrinfo(NULL, port, &hints, &result);
    
    if(ret){
        fprintf(stderr, "getaddrinfo error: %d\n", ret);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    SOCKET listen_sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(listen_sock == INVALID_SOCKET){
        fprintf(stderr, "socket error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    ret = bind(listen_sock, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);

    if(ret == SOCKET_ERROR){
        fprintf(stderr, "bind error: %ld\n", WSAGetLastError());
        closesocket(listen_sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    ret = listen(listen_sock, SOMAXCONN);
    if(ret == SOCKET_ERROR){
        fprintf(stderr, "listen error: %ld\n", WSAGetLastError());
        closesocket(listen_sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    
    // create the thread to do the accepts
    HANDLE connecter_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&connecter, &listen_sock, 0, NULL);
    if(!connecter_thread){
        fprintf(stderr, "CreateThread error: %ld\n", GetLastError());
        closesocket(listen_sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }    
    
    while(!ready()) Sleep(500);
    
    ret = TerminateThread(connecter_thread, 0);
    closesocket(listen_sock);

    if(!ret){
        fprintf(stderr, "TerminateThread error: ", GetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    run_server();
}

