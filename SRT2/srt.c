#include <gst/gst.h>

int main(int argc, char* argv[]) {
    GstElement* pipeline, * source, * filter, * convert, * encoder, * muxer, * sink;
    GstCaps* caps;
    GstBus* bus;
    GstMessage* msg;

    gst_init(&argc, &argv);

    pipeline = gst_pipeline_new("meu_pipeline");

    source = gst_element_factory_make("videotestsrc", "source");
    filter = gst_element_factory_make("capsfilter", "filter");
    convert = gst_element_factory_make("videoconvert", "convert");
    encoder = gst_element_factory_make("x264enc", "encoder");
    muxer = gst_element_factory_make("mpegtsmux", "muxer");
    sink = gst_element_factory_make("srtsink", "sink");

    if (!pipeline || !source || !filter || !convert || !encoder || !muxer || !sink) {
        g_printerr("Não foi possível criar um ou mais elementos.\n");
        return -1;
    }

    caps = gst_caps_new_simple("video/x-raw",
        "height", G_TYPE_INT, 1080,
        "width", G_TYPE_INT, 1920,
        NULL);
    g_object_set(G_OBJECT(filter), "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(G_OBJECT(encoder), "tune", "zerolatency", NULL);

    caps = gst_caps_new_simple("video/x-h264",
        "profile", G_TYPE_STRING, "high",
        NULL);
    g_object_set(G_OBJECT(encoder), "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(G_OBJECT(sink), "uri", "srt://:8888", NULL);

    gst_bin_add_many(GST_BIN(pipeline), source, filter, convert, encoder, muxer, sink, NULL);
    if (!gst_element_link_many(source, filter, convert, encoder, muxer, sink, NULL)) {
        g_printerr("Os elementos não puderam ser vinculados.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    g_print("Iniciado\n");
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_print("Setado PLAYING\n");
    /* Espere até que ocorra erro ou EOS */
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    g_usleep(60000000);
    /* Libere os recursos */
    if (msg != NULL)
        g_print("mensagem");
    {
        gst_message_unref(msg);
        gst_object_unref(bus);
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }
    return 0;
    // ... (execute o pipeline)

    gst_object_unref(pipeline);
    return 0;
}