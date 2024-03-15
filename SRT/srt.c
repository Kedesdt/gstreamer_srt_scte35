#include <gst/gst.h>
#include <gio/gio.h>

#define IP_PORT "srt://:8888"

void callback(GstElement* gstsrtsink,
    gint unused,
    GSocketAddress* addr,
    gpointer udata)
{
    g_print("CALLBACK");
}

gboolean
caller_connecting_callback(GstElement* gstsrtsink,
    GSocketAddress* addr,
    gchararray stream_id,
    gpointer udata)
{
    g_print("Connecting....");
    return TRUE;
}

int main(int argc, char* argv[]) {
    GstElement* pipeline, * source, * encoder, * muxer, * sink;
    GstBus* bus;
    GstMessage* msg;

    /* Inicialize o GStreamer */
    gst_init(&argc, &argv);

    GError *err = NULL;

    /* Crie os elementos */
    source = gst_element_factory_make("videotestsrc", "source");
    encoder = gst_element_factory_make("x264enc", "encoder");
    muxer = gst_element_factory_make("mpegtsmux", "muxer");
    sink = gst_element_factory_make("srtsink", "sink");
    //sink = gst_element_make_from_uri(GST_URI_SINK, "srt://127.0.0.1:8888", "sink", &err);

    /* Verifique se os elementos foram criados corretamente */
    if (!source || !encoder || !muxer || !sink) {
        g_printerr("Não foi possível criar os elementos.\n");
        return -1;
    }

    /* Configure o srtsink */
    g_object_set(sink, "uri", IP_PORT, NULL);
    g_object_set(sink, "mode", 2, NULL);
    //g_object_set(sink, "wait-for-connection", TRUE, NULL);

    g_signal_connect(sink, "caller-added", G_CALLBACK(callback), NULL);
    g_signal_connect(sink, "caller_connecting", G_CALLBACK(caller_connecting_callback), NULL);
    

    /* Crie o pipeline */
    pipeline = gst_pipeline_new("test-pipeline");

    if (!pipeline) {
        g_printerr("Não foi possível criar o pipeline.\n");
        return -1;
    }

    /* Construa o pipeline */
    gst_bin_add_many(GST_BIN(pipeline), source, encoder, muxer, sink, NULL);
    if (gst_element_link_many(source, encoder, muxer, sink, NULL) != TRUE) {
        g_printerr("Os elementos não puderam ser vinculados.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Inicie a reprodução */
    g_print("Iniciado");
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_print("Setado PLAYING");
    /* Espere até que ocorra erro ou EOS */
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    g_usleep(60000000);
    /* Libere os recursos */
    if (msg != NULL)
    {
        gst_message_unref(msg);
        gst_object_unref(bus);
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }
    return 0;
}