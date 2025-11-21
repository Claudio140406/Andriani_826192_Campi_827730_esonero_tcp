/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP server
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

#define NO_ERROR 0

void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}


// Lista città supportate
const char *citta[] = {
    "Bari", "Roma", "Milano", "Napoli", "Torino",
    "Palermo", "Genova", "Bologna", "Firenze", "Venezia"
};
const int num_cities = sizeof(citta) / sizeof(citta[0]);

// Validazione città (case-insensitive)
int citta_valida(const char *c) {
    for (int i = 0; i < num_cities; i++) {
        if (strcasecmp(c, citta[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Funzioni di generazione dati meteo
float get_temperature(void) {
    return ((float)rand() / RAND_MAX) * (40.0 - (-10.0)) + (-10.0);
}
float get_humidity(void) {
    return ((float)rand() / RAND_MAX) * (100.0 - 20.0) + 20.0;
}
float get_wind(void) {
    return ((float)rand() / RAND_MAX) * (100.0 - 0.0) + 0.0;
}
float get_pressure(void) {
    return ((float)rand() / RAND_MAX) * (1050.0 - 950.0) + 950.0;
}


int main(int argc, char *argv[]) {

    // TODO: Implement server logic

#if defined WIN32
    // Initialize Winsock
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (result != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return 0;
    }
#endif

    // Porta di default
    int port = SERVER_PORT;
    // Parsing argomenti da riga di comando
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
            port = atoi(argv[++i]);
        }
    }

    int my_socket;

    // TODO: Create socket
    my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (my_socket < 0) {
        perror("Creazione socket fallita");
        clearwinsock();
        exit(EXIT_FAILURE);
    }

    // TODO: Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // TODO: Bind socket
    if (bind(my_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() fallita");
        closesocket(my_socket);
        clearwinsock();
        exit(EXIT_FAILURE);
    }

    // TODO: Set socket to listen
    if (listen(my_socket, QUEUE_SIZE) < 0) {
        perror("listen() fallita");
        closesocket(my_socket);
        clearwinsock();
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d...\n", port);

    // TODO: Implement connection acceptance loop
    while (1) {
        struct sockaddr_in client_addr;
#if defined WIN32
        int client_len = sizeof(client_addr);
#else
        socklen_t client_len = sizeof(client_addr);
#endif
        int client_socket = accept(my_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("accept() fallita");
            continue;
        }

        printf("Connessione accettata da %s\n", inet_ntoa(client_addr.sin_addr));

        weather_request_t req;
        weather_response_t res;

        // Ricezione richiesta
        if (recv(client_socket, (char*)&req, sizeof(req), 0) <= 0) {
            perror("Errore recv");
            closesocket(client_socket);
            continue;
        }

        printf("Richiesta '%c %s' dal client ip %s\n", req.type, req.city, inet_ntoa(client_addr.sin_addr));

        // Validazione richiesta
        if (req.type != 't' && req.type != 'h' && req.type != 'w' && req.type != 'p') {
            res.status = STATUS_INVALID_REQUEST;
            res.type = '\0';
            res.value = 0.0;
        } else if (!citta_valida(req.city)) {
            res.status = STATUS_CITY_NOT_AVAILABLE;
            res.type = '\0';
            res.value = 0.0;
        } else {
            res.status = STATUS_SUCCESS;
            res.type = req.type;
            switch (req.type) {
                case 't': res.value = get_temperature(); break;
                case 'h': res.value = get_humidity(); break;
                case 'w': res.value = get_wind(); break;
                case 'p': res.value = get_pressure(); break;
            }
        }

        // Invio risposta
        send(client_socket, (char*)&res, sizeof(res), 0);

        // Chiusura connessione client
        closesocket(client_socket);
    }

    printf("Server terminated.\n");

    closesocket(my_socket);
    clearwinsock();
    return 0;
} // main end
