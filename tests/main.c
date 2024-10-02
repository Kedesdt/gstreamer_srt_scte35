#include <stdio.h>
#include <gst/gst.h>

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);
    g_print("V1");

    // Crie os elementos da pipeline
    GstElement* pipeline = gst_pipeline_new("my-pipeline");
    GstElement* src = gst_element_factory_make("audiotestsrc", "src");
    GstElement* convert = gst_element_factory_make("audioconvert", "convert");
    GstElement* encode = gst_element_factory_make("lamemp3enc", "encode");
    GstElement* parse = gst_element_factory_make("mpegaudioparse", "parse");
    GstElement* mux = gst_element_factory_make("mpegtsmux", "mux");
    GstElement* pay = gst_element_factory_make("rtpmp2tpay", "pay");
    GstElement* sink = gst_element_factory_make("udpsink", "sink");

    g_print("Elementos criados\n");

    // Configure os elementos (defina os parâmetros necessários)
    g_object_set(G_OBJECT(src), "wave", 0, NULL);
    g_print("parametro wave\n");
    g_object_set(G_OBJECT(encode), "bitrate", 128, NULL);
    g_print("bitrate\n");
    g_object_set(G_OBJECT(sink), "host", "127.0.0.1", "port", 1234, NULL);
    g_print("udp\n");

    // Adicione os elementos à pipeline
    gst_bin_add_many(GST_BIN(pipeline), src, convert, encode, parse, mux, pay, sink, NULL);
    g_print("Adicionados a pipeline");
    // Linke os elementos
    gst_element_link_many(src, convert, encode, parse, mux, pay, sink, NULL);
    g_print("Linkados");
    // Execute a pipeline
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_print("Falha ao iniciar a pipeline\n");
        return -1;
    }

    while (1) {
        g_print("%d", gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE));
        g_usleep(500000);

    }
    
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}