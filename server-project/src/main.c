/*
 * server.c
 *
 * TCP Server - Servizio Meteo
 * Portabile su Windows, Linux e macOS
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#define strcasecmp _stricmp
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "protocol.h"

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

void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

int main() {
#if defined WIN32
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != NO_ERROR) {
        printf("Errore a WSAStartup()\n");
        exit(EXIT_FAILURE);
    }
#endif

    srand(time(NULL));

    // Creazione socket
    int my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (my_socket < 0) {
        printf("Creazione socket fallita.\n");
        clearwinsock();
        exit(EXIT_FAILURE);
    }

    // Configurazione indirizzo
    struct sockaddr_in sad;
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr("127.0.0.1");
    sad.sin_port = htons(SERVER_PORT);

    // Bind
    if (bind(my_socket, (struct sockaddr*)&sad, sizeof(sad)) < 0) {
        printf("bind() fallita.\n");
        closesocket(my_socket);
        clearwinsock();
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(my_socket, QUEUE_SIZE) < 0) {
        printf("listen() fallita.\n");
        closesocket(my_socket);
        clearwinsock();
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d...\n", SERVER_PORT);

    // Ciclo infinito per gestire i client
    while (1) {
        struct sockaddr_in cad;
        int client_len = sizeof(cad);
        int client_socket = accept(my_socket, (struct sockaddr*)&cad, &client_len);
        if (client_socket < 0) {
            printf("accept() fallita.\n");
            continue;
        }

        printf("Connessione accettata da %s\n", inet_ntoa(cad.sin_addr));

        weather_request_t rich;
        weather_response_t ris;

        // Ricezione richiesta
        if (recv(client_socket, (char*)&rich, sizeof(rich), 0) <= 0) {
            perror("Errore recv");
            closesocket(client_socket);
            continue;
        }

        printf("Richiesta '%c %s' dal client ip %s\n", rich.type, rich.city, inet_ntoa(cad.sin_addr));

        // Validazione richiesta
        if (rich.type != 't' && rich.type != 'h' && rich.type != 'w' && rich.type != 'p') {
            ris.status = STATUS_INVALID_REQUEST;
            ris.type = '\0';
            ris.value = 0.0;
        } else if (!citta_valida(rich.city)) {
            ris.status = STATUS_CITY_NOT_AVAILABLE;
            ris.type = '\0';
            ris.value = 0.0;
        } else {
            ris.status = STATUS_SUCCESS;
            ris.type = rich.type;
            switch (rich.type) {
                case 't': ris.value = get_temperature(); break;
                case 'h': ris.value = get_humidity(); break;
                case 'w': ris.value = get_wind(); break;
                case 'p': ris.value = get_pressure(); break;
            }
        }

        // Invio risposta
        send(client_socket, (char*)&ris, sizeof(ris), 0);

        // Chiusura connessione client
        closesocket(client_socket);
    }

    // Chiusura socket server
    closesocket(my_socket);
    clearwinsock();
    return 0;
}
