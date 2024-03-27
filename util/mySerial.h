
#ifndef MYSERIAL_H
#define MYSERIAL_H

#include <stdbool.h>
#include <windows.h> // Para HANDLE e DWORD
#include <gst/gst.h>


typedef void (*funcao)(GstElement*, gboolean);

// Definição da estrutura MySerial
struct MySerial {
    HANDLE hComm;         // Alça para a porta serial
    DWORD dwModemStatus;  // Status do modem
    DCB dcb;              // Configurações da porta
    int Status;           // Outro atributo (você pode adicionar mais detalhes aqui)

};

// Métodos (funções) associados à estrutura
void init(struct MySerial*, const char* com);   // 
void destroy(struct MySerial*);
void setRts(struct MySerial*, bool value); // método para definir RTS
void setDtr(struct MySerial*, bool value); // método para definir DTR
bool getDsr(struct MySerial*); // método para obter DSR
bool getCts(struct MySerial*); // método para obter CTS

void* serial_loop(funcao, struct MySerial*, GstElement*);
void* loop(void*);


#endif