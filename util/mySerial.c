//#include <windows.h>
//#include <stdio.h>
#include "mySerial.h"
//#include <jansson.h>
//#include <string>
#include <pthread.h>
//#include <gst/gst.h>
#include <stdlib.h>


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

typedef void (*funcao)(GstElement*, gboolean);

struct Param {
    funcao f;
    struct MySerial* serial;
    GstElement* muxer;
};

void* loop(void* args) {

    struct Param* params = (struct Param*)args;

    while (1) {

        g_usleep(5000);
        if (getDsr(params->serial)) {

            params->f(params->muxer, FALSE);
            g_usleep(2000000);
        }
        if (getCts(params->serial)) {
            params->f(params->muxer, TRUE);
            g_usleep(2000000);
        }
    }
    return NULL;
}

void* serial_loop(funcao f, struct MySerial* serial, GstElement * muxer) {
    pthread_t thread_id;
    struct Param* params = (struct Param*)malloc(sizeof(struct Param));
    
    params->f = f;
    params->serial = serial;
    params->muxer = muxer;

    if (pthread_create(&thread_id, NULL, loop, params) != 0) {
        g_print("Erro ao criar thread");
    }
    else {
        g_print("Loop Serial Iniciado");
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

void init(struct MySerial* serial, const LPCWSTR com) {

    g_print("Abrindo Porta Serial.\n");
    // Abra a porta serial
    serial->hComm = CreateFile(com, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    g_print("Aberta\n");
    if (serial->hComm == INVALID_HANDLE_VALUE) {
        g_print("Erro ao abrir a porta serial.\n");
        return;
    }
    // Obtenha o status do modem
    if (!GetCommModemStatus(serial->hComm, &serial->dwModemStatus)) {
        g_print("Erro ao obter o status do modem.\n");
        CloseHandle(serial->hComm);
        return;
    }
    SecureZeroMemory(&serial->dcb, sizeof(DCB));
    serial->dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(serial->hComm, &serial->dcb)) {
        g_print("Erro ao obter o estado da porta serial.\n");
        CloseHandle(serial->hComm);
        return;
    }

}

void setRts(struct MySerial* serial, bool value) {
    if (value) {
        serial->dcb.fRtsControl = RTS_CONTROL_ENABLE;
        g_print("RTS SET ENABLE\n");
    }
    else {
        serial->dcb.fRtsControl = RTS_CONTROL_DISABLE;
        g_print("RTS SET DISABLE\n");
    }

    if (!SetCommState(serial->hComm, &serial->dcb)) {
        g_print("Erro ao configurar a porta serial.\n");
        CloseHandle(serial->hComm);

    }
}

void setDtr(struct MySerial* serial, bool value) {
    if (value)
        serial->dcb.fDtrControl = DTR_CONTROL_ENABLE;
    else
        serial->dcb.fDtrControl = DTR_CONTROL_DISABLE;
    if (!SetCommState(serial->hComm, &serial->dcb)) {
        g_print("Erro ao configurar a porta serial.\n");
        CloseHandle(serial->hComm);
    }
}

bool getDsr(struct MySerial* serial) {

    if (!GetCommModemStatus(serial->hComm, &serial->dwModemStatus)) {
        g_print("Erro ao obter o status do modem.\n");
        CloseHandle(serial->hComm);
        return 0;
    }
    return (serial->dwModemStatus & MS_DSR_ON) != 0;
}

bool getCts(struct MySerial* serial) {

    if (!GetCommModemStatus(serial->hComm, &serial->dwModemStatus)) {
        g_print("Erro ao obter o status do modem.\n");
        CloseHandle(serial->hComm);
        return 0;
    }
    return (serial->dwModemStatus & MS_CTS_ON) != 0;
}

void destroy(struct MySerial* serial) {
    CloseHandle(serial->hComm);
    free(serial);
}


