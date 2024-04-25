#pragma once
#include <gst/gst.h>
#ifdef __cplusplus
extern "C" {
#endif //  Para funcionar no C++

    struct Config {
        wchar_t* com;
        char* uri;
        char* device;
        int running;
        int stop;
    };

    int srt_main(struct Config*);
    static void send_splice(GstElement*, gboolean);
    static gboolean send_splice_in(GstElement*);
    static gboolean send_splice_out(GstElement*);
    static int configuration(struct Config*);

#ifdef __cplusplus
}
#endif // Para funcionar no C++