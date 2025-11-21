/*
 * protocol.h
 *
 * Header file condiviso tra client e server
 * Definizioni, costanti, strutture e prototipi
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdbool.h>

// Parametri condivisi
#define SERVER_PORT 56700   // Porta di default del server
#define BUFFER_SIZE 512     // Dimensione buffer per messaggi
#define QUEUE_SIZE 5        // Dimensione della coda di connessioni pendenti
#define NO_ERROR 0

// Codici di stato della risposta
#define STATUS_SUCCESS 0
#define STATUS_CITY_NOT_AVAILABLE 1
#define STATUS_INVALID_REQUEST 2

// Struttura richiesta (Client → Server)
typedef struct {
    char type;        // Weather data type: 't', 'h', 'w', 'p'
    char city[64];    // Nome città (null-terminated string)
} weather_request_t;

// Struttura risposta (Server → Client)
typedef struct {
    unsigned int status;  // Codice di stato
    char type;            // Eco del tipo richiesto
    float value;          // Valore meteo
} weather_response_t;

// Funzioni di generazione dati meteo (server)
float get_temperature(void);    // Range: -10.0 to 40.0 °C
float get_humidity(void);       // Range: 20.0 to 100.0 %
float get_wind(void);           // Range: 0.0 to 100.0 km/h
float get_pressure(void);       // Range: 950.0 to 1050.0 hPa

// Funzione di validazione città (server)
int citta_valida(const char *c);

#endif /* PROTOCOL_H_ */
