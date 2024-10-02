#pragma once
#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif //Para funcionar no C++

    struct Config {
        wchar_t* com    ;
        char*    uri    ;
        char*    device ;
        int*     running;
        int*     stop   ;
        int*     mode   ;
    };

    void srt_thread(struct Config*);
    void* init_srt_client(void*);
    int init_srt(struct Config*);
    int srt_client_main(struct Config*);
    int init_srt(struct Config*);
    void stop_loop();
#ifdef __cplusplus
}
#endif // Fechamento das chaves abertas na linha 5