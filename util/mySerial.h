#pragma once

#include <stdbool.h>
#include <windows.h> // Para HANDLE e DWORD
#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif //Para funcionar em c++

    typedef void (*funcao)(GstElement*, gboolean);

    // Defini��o da estrutura MySerial
    struct MySerial {
        HANDLE hComm;         // Al�a para a porta serial
        DWORD dwModemStatus;  // Status do modem
        DCB dcb;              // Configura��es da porta
        int Status;           // Outro atributo (voc� pode adicionar mais detalhes aqui)
        int running;

    };

    // M�todos (fun��es) associados � estrutura
    void init(struct MySerial*, const LPCWSTR com);   //
    void destroy(struct MySerial*);
    void setRts(struct MySerial*, int value); // m�todo para definir RTS
    void setDtr(struct MySerial*, int value); // m�todo para definir DTR
    int getDsr(struct MySerial*); // m�todo para obter DSR
    int getCts(struct MySerial*); // m�todo para obter CTS

    void* serial_loop(funcao, struct MySerial*, GstElement*);
    void* loop(void*);

#ifdef __cplusplus
}
#endif // Fechamento das chaves apertas na linha 8