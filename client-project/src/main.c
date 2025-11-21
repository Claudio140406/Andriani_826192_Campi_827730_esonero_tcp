/*
 * main.c
 *
 * TCP Client - Servizio Meteo
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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "protocol.h"

#define NO_ERROR 0

void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

int set_Socket() {
    int socket_value = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_value < 0) {

        perror("Creazione del socket fallita.\n");
        clearwinsock();
        exit(EXIT_FAILURE);
    }
    return socket_value;
}

int try_Server_Connection(int socket , struct sockaddr_in sad, size_t size_sad) {
    if (connect(socket, (struct sockaddr*)&sad, size_sad) < 0) {

        printf("Connessione al server fallita.\n");
        closesocket(socket);
        clearwinsock();
        exit(EXIT_FAILURE);
    }
    return 0;
}

struct sockaddr_in set_Sockaddr_in(const char* Ip_addr, int port) {
    struct sockaddr_in x;
    x.sin_family = AF_INET;
    x.sin_addr.s_addr = inet_addr(Ip_addr);
    x.sin_port = htons(port);
    return x;
}

void normalize_city(char *city) {
    if (city == NULL || city[0] == '\0') return;
    city[0] = toupper((unsigned char)city[0]);
    for (int i = 1; city[i] != '\0'; i++) {
        city[i] = tolower((unsigned char)city[i]);
    }
}

void Request_Sending(int my_socket, weather_request_t* request, size_t size_req) {
    if (send(my_socket, (char*)request, size_req, 0) < 0) {

        printf("Invio della richiesta fallito.\n");
        closesocket(my_socket);
        clearwinsock();
        exit(EXIT_FAILURE);
    }
}

void Data_Recived(int my_socket, weather_response_t* response, size_t size_res) {
    int bytes_received = recv(my_socket, (char*)response, size_res, 0);
    if (bytes_received <= 0) {

        printf("Ricezione della risposta fallita.\n");
        closesocket(my_socket);
        clearwinsock();
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
#if defined WIN32
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (result != NO_ERROR) {

        printf("Errore a WSAStartup()\n");
        exit(EXIT_FAILURE);
    }
#endif

    const char* IP_address = "127.0.0.1";
    int port = SERVER_PORT;
    weather_request_t weather_request;
    weather_response_t weather_response;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i+1 < argc) {
            IP_address = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-r") == 0 && i+1 < argc) {
            char *req = argv[++i];
            weather_request.type = req[0];
            strncpy(weather_request.city, req+2, sizeof(weather_request.city)-1);
            weather_request.city[sizeof(weather_request.city)-1] = '\0';
            normalize_city(weather_request.city);
        }
    }

    int my_socket = set_Socket();
    struct sockaddr_in sad = set_Sockaddr_in(IP_address, port);

    try_Server_Connection(my_socket, sad, sizeof(sad));

    Request_Sending(my_socket, &weather_request, sizeof(weather_request));
    Data_Recived(my_socket, &weather_response, sizeof(weather_response));

    printf("Ricevuto risultato dal server ip %s. ", IP_address);
    if (weather_response.status == STATUS_SUCCESS) {
        if (weather_response.type == 't')
            printf("%s: Temperatura = %.1f°C\n", weather_request.city, weather_response.value);
        else if (weather_response.type == 'h')
            printf("%s: Umidità = %.1f%%\n", weather_request.city, weather_response.value);
        else if (weather_response.type == 'w')
            printf("%s: Vento = %.1f km/h\n", weather_request.city, weather_response.value);
        else if (weather_response.type == 'p')
            printf("%s: Pressione = %.1f hPa\n", weather_request.city, weather_response.value);
    } else if (weather_response.status == STATUS_CITY_NOT_AVAILABLE) {
        printf("Città non disponibile\n");
    } else if (weather_response.status == STATUS_INVALID_REQUEST) {
        printf("Richiesta non valida\n");
    }

    closesocket(my_socket);
    clearwinsock();

    return 0;
} // main end
