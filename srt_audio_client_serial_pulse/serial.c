#include <windows.h>
#include <stdio.h>
#include "serial.h"
#include <jansson.h>

int init() {
    
    HANDLE hComm;
    DWORD dwModemStatus;
    int Status;

    // Abra a porta serial
    hComm = CreateFile("\\\\.\\COM1", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (hComm == INVALID_HANDLE_VALUE) {
        printf("Erro ao abrir a porta serial.\n");
        return 1;
    }

    // Obtenha o status do modem
    if (!GetCommModemStatus(hComm, &dwModemStatus)) {
        printf("Erro ao obter o status do modem.\n");
        CloseHandle(hComm);
        return 1;
    }

    // Verifique os estados dos pinos
    printf("DCD: %s\n", (dwModemStatus & MS_RLSD_ON) ? "ON" : "OFF");
    printf("DSR: %s\n", (dwModemStatus & MS_DSR_ON) ? "ON" : "OFF");
    printf("CTS: %s\n", (dwModemStatus & MS_CTS_ON) ? "ON" : "OFF");
    printf("RI: %s\n", (dwModemStatus & MS_RING_ON) ? "ON" : "OFF");

    // Feche a porta serial
    CloseHandle(hComm);

    return 0;
}