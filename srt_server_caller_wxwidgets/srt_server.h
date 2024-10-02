#pragma once
#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif //Para funcionar no C++

    struct Config {
        wchar_t* com;
        char* uri;
        char* device;
        int* running;
        int* stop;
        int* mode;
    };

    //struct Config* srt_server();
    void srt_server(struct Config*);
    void* init_srt_server(void*);
    int init_srt(struct Config*);
    int srt_main(struct Config*);
    static void send_splice(GstElement*, gboolean);
    static gboolean send_splice_in(GstElement*);
    static gboolean send_splice_out(GstElement*);
    static int configuration(struct Config*);
    int init_srt(struct Config*);
#ifdef __cplusplus
}
#endif // Fechamento das chaves abertas na linha 5

