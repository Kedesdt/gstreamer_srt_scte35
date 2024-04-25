#pragma once

#include <stdbool.h>
#include <windows.h> // Para HANDLE e DWORD
#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif //Para funcionar em c++

    typedef void (*funcao)(GstElement*, gboolean);

    // Definição da estrutura MySerial
    struct MySerial {
        HANDLE hComm;         // Alça para a porta serial
        DWORD dwModemStatus;  // Status do modem
        DCB dcb;              // Configurações da porta
        int Status;           // Outro atributo (você pode adicionar mais detalhes aqui)
        int running;

    };

    // Métodos (funções) associados à estrutura
    void init(struct MySerial*, const LPCWSTR com);   //
    void destroy(struct MySerial*);
    void setRts(struct MySerial*, int value); // método para definir RTS
    void setDtr(struct MySerial*, int value); // método para definir DTR
    int getDsr(struct MySerial*); // método para obter DSR
    int getCts(struct MySerial*); // método para obter CTS

    void* serial_loop(funcao, struct MySerial*, GstElement*);
    void* loop(void*);

#ifdef __cplusplus
}
#endif // Fechamento das chaves apertas na linha 8