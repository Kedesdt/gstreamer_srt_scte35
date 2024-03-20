#include <windows.h>
#include <stdio.h>
#include "serial.h"
#include <jansson.h>
#include <string>
#include <pthread.h>

/*
* Serial pinout
* 
* 1 -   CD      INPUT
* 2 -   RXD     INPUT
* 3 -   TXD     OUTPUT
* 4 -   DTR     OUTPUT
* 5 -   SG      GROUND
* 6 -   DSR     INPUT
* 7 -   RTS     OUTPUT
* 8 -   CTS     INPUT
* 9 -   VCC     INPUT
* 
* IN: DSR e CTS
* OUT: DTR e RTS
* 
*/

typedef void (*func)(GstElement*, gboolean);

void* serial_loop((*sendSplice) *func) {
    while (1) {
        // Coloque aqui o código da sua função
        printf("Executando minha função...\n");
        usleep(5000); // Espera por 5 ms
    }
    return NULL;
}


// Função para converter char* para LPCWSTR
LPCWSTR convertCharArrayToLPCWSTR(const char* charArray) {
    const char* cs = charArray;
    wchar_t string[4096] = { 0 }; // Inicialize com zeros

    // Converta a sequência de caracteres multibyte para wide string
    MultiByteToWideChar(CP_ACP, 0, cs, -1, string, 4096);

    // Retorne o ponteiro para a wide string
    return string;
}

void init(struct MySerial* serial, const char* com) {

    // Abra a porta serial
    serial->hComm = CreateFile(convertCharArrayToLPCWSTR(com), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (serial->hComm == INVALID_HANDLE_VALUE) {
        printf("Erro ao abrir a porta serial.\n");
        return;
    }
    // Obtenha o status do modem
    if (!GetCommModemStatus(serial->hComm, &serial->dwModemStatus)) {
        printf("Erro ao obter o status do modem.\n");
        CloseHandle(serial->hComm);
        return;
    }
    SecureZeroMemory(&serial->dcb, sizeof(DCB));
    serial->dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(serial->hComm, &serial->dcb)) {
        printf("Erro ao obter o estado da porta serial.\n");
        CloseHandle(serial->hComm);
        return;
    }

}

void setRts(struct MySerial* serial, bool value) {
    if (value)
        serial->dcb.fRtsControl = RTS_CONTROL_ENABLE;
    else
        serial->dcb.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(serial->hComm, &serial->dcb)) {
        printf("Erro ao configurar a porta serial.\n");
        CloseHandle(serial->hComm);

    }
}

void setDtr(struct MySerial* serial, bool value) {
    if (value)
        serial->dcb.fDtrControl = DTR_CONTROL_ENABLE;
    else
        serial->dcb.fDtrControl = DTR_CONTROL_DISABLE;
    if (!SetCommState(serial->hComm, &serial->dcb)) {
        printf("Erro ao configurar a porta serial.\n");
        CloseHandle(serial->hComm);
    }
}

bool getDsr(struct MySerial* serial) {
    return serial->dwModemStatus & MS_DSR_ON != 0;
}

bool getCts(struct MySerial* serial) {
    return serial->dwModemStatus & MS_CTS_ON != 0;
}



    
