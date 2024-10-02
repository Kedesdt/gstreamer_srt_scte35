#include <gst/gst.h>

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    // Crie o pipeline
    GstElement* pipeline = gst_pipeline_new("udp-receiver");

    // Elementos
    GstElement* udpsrc = gst_element_factory_make("udpsrc", "udpsrc");
    GstElement* tsparse = gst_element_factory_make("tsparse", "tsparse");
    GstElement* tsdemux = gst_element_factory_make("tsdemux", "tsdemux");
    GstElement* h264parse = gst_element_factory_make("h264parse", "h264parse");
    GstElement* avdec_h264 = gst_element_factory_make("avdec_h264", "avdec_h264");
    GstElement* videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    GstElement* ximagesink = gst_element_factory_make("ximagesink", "ximagesink");

    if (!pipeline || !udpsrc || !tsparse || !tsdemux || !h264parse || !avdec_h264 || !videoconvert || !ximagesink) {
        g_printerr("Erro ao criar elementos.\n");
        return -1;
    }

    // Configuração do udpsrc
    g_object_set(udpsrc, "port", 5000, NULL);

    // Adicione os elementos ao pipeline
    gst_bin_add_many(GST_BIN(pipeline), udpsrc, tsparse, tsdemux, h264parse, avdec_h264, videoconvert, ximagesink, NULL);

    // Conecte os elementos
    gst_element_link_many(udpsrc, tsparse, tsdemux, h264parse, avdec_h264, videoconvert, ximagesink, NULL);

    // Execute o pipeline
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Falha ao iniciar o pipeline.\n");
        return -1;
    }

    g_print("Recebendo o fluxo via UDP (127.0.0.1:5000)...\n");
    gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);

    // Encerre o pipeline
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
