#include <gst/gst.h>

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    // Crie o pipeline
    GstElement* pipeline = gst_pipeline_new("mpegts-pipeline");

    // Elementos
    GstElement* mpegtsmux = gst_element_factory_make("mpegtsmux", "muxer");
    GstElement* udpsink = gst_element_factory_make("udpsink", "udpsink");
    GstElement* videotestsrc = gst_element_factory_make("videotestsrc", "video-src");
    GstElement* audiotestsrc = gst_element_factory_make("audiotestsrc", "audio-src");

    if (!pipeline || !mpegtsmux || !udpsink || !videotestsrc || !audiotestsrc) {
        g_printerr("Erro ao criar elementos.\n");
        return -1;
    }

    // Configuração do udpsink
    g_object_set(udpsink, "host", "127.0.0.1", "port", 5000, NULL);

    // Crie uma tabela PAT e adicione um programa
    GstElement* pat = gst_element_factory_make("mpegtsmux", "pat");
    GstElement* program = gst_element_factory_make("mpegtspatprogram", "program");
    g_object_set(program, "program-number", 1, "pid", 256, NULL); // Exemplo: PID 256

    // Adicione o programa à tabela PAT
    gst_bin_add_many(GST_BIN(pipeline), mpegtsmux, udpsink, pat, program, videotestsrc, audiotestsrc, NULL);
    gst_element_link_many(pat, mpegtsmux, udpsink, NULL);
    gst_element_link_many(videotestsrc, mpegtsmux, NULL);
    gst_element_link_many(audiotestsrc, mpegtsmux, NULL);

    // Execute o pipeline
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Falha ao iniciar o pipeline.\n");
        return -1;
    }

    g_print("Streaming via UDP (127.0.0.1:5000)...\n");
    gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);

    // Encerre o pipeline
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}

