#ifndef MYSERIAL_H
#define MYSERIAL_H

#include <stdbool.h>
#include <windows.h> // Para HANDLE e DWORD
#include <gst/gst.h>


typedef void (*func)(GstElement*, gboolean);

// Defini��o da estrutura MySerial
struct MySerial {
    HANDLE hComm;         // Al�a para a porta serial
    DWORD dwModemStatus;  // Status do modem
    DCB dcb;              // Configura��es da porta
    int Status;           // Outro atributo (voc� pode adicionar mais detalhes aqui)

};

// M�todos (fun��es) associados � estrutura
void init(struct MySerial*, const char* com);   // 
void destroy(struct MySerial*);
void setRts(struct MySerial*, bool value); // m�todo para definir RTS
void setDtr(struct MySerial*, bool value); // m�todo para definir DTR
bool getDsr(struct MySerial*); // m�todo para obter DSR
bool getCts(struct MySerial*); // m�todo para obter CTS

void *serial_loop(func, struct MySerial*, GstElement*);
void *loop(void*);


#endif