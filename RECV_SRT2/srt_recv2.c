#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
//#include <glib.h>


static void on_scte_35(GstElement* demux, guint pid, GstMpegtsSection* section, gpointer user_data)
{
    /* Esta função será chamada sempre que uma mensagem SCTE-35 for emitida pelo tsparse */
    g_print(pid);
    g_print("SCTE35");
    /* Aqui você pode adicionar seu código para processar a mensagem SCTE-35 */
}

static void on_pad_added(GstElement* element, GstPad* pad, gpointer data) {
    GstPad* sinkpad;
    GstElement* sink = (GstElement*)data;

    sinkpad = gst_element_get_static_pad(sink, "sink");

    if (!gst_pad_is_linked(sinkpad)) {
        if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK)
            g_error("Failed to link elements");
    }

    gst_object_unref(sinkpad);
}

static gboolean bus_call(GstBus* bus, GstMessage* msg, gpointer data) {
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ELEMENT: {

        g_print(".");
        const GstStructure* s = gst_message_get_structure(msg);

        gchar* s_str = gst_structure_to_string(s);
        g_print("%s\n", s_str);
        g_free(s_str);

        //g_print(s->name);
        if (gst_structure_has_name(s, "mpegts-section")) {

            g_print("MSG");
            // Esta é uma mensagem SCTE-35
            // Aqui você pode adicionar seu código para processar a mensagem SCTE-35
        }
        break;
    }
    default:
        break;
    }

    return TRUE;
}

int main(int argc, char* argv[]) {
    GstElement* pipeline, * demux, *tsparse, *source, *decodebin, *sink;
    GstBus* bus;

    gst_init(&argc, &argv);

    pipeline = gst_pipeline_new("scte35-pipeline");
    demux = gst_element_factory_make("tsdemux", "demux");
    tsparse = gst_element_factory_make("tsparse", "tsparse");

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, NULL);
    gst_object_unref(bus);

    source = gst_element_factory_make("srtclientsrc", "source");
    decodebin = gst_element_factory_make("decodebin", "decodebin");
    sink = gst_element_factory_make("autovideosink", "sink");

    if (!pipeline || !source || !demux || !decodebin || !sink || !tsparse) {
        g_printerr("Não foi possível criar um ou mais elementos.\n");
        return -1;
    }

    g_object_set(G_OBJECT(source), "uri", "srt://127.0.0.1:8888", NULL);

    gst_bin_add_many(GST_BIN(pipeline), source, tsparse, demux, decodebin, sink, NULL);
    if (!gst_element_link_many(source, tsparse, demux, NULL)) {
        g_printerr("Os elementos não puderam ser vinculados.\n");
        gst_object_unref(pipeline);
        return -1;
    }
    g_signal_connect(demux, "pad-added", G_CALLBACK(on_pad_added), decodebin);
    g_signal_connect(decodebin, "pad-added", G_CALLBACK(on_pad_added), sink);
    g_signal_connect(demux, "section-added", G_CALLBACK(on_scte_35), NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    //g_usleep(50000000);

    // Entre no loop principal
    GMainLoop* loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    // Limpeza
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}