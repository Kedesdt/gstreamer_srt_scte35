#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>

#define GST_USE_UNSTABLE_API

static void
send_splice(GstElement* mux, gboolean out)
{
    GstMpegtsSCTESIT* sit;
    GstMpegtsSection* section;

    g_print("Sending Splice %s event\n", out ? "Out" : "In");

    /* Splice is at 5s for 30s */
    if (out)
        sit = gst_mpegts_scte_splice_out_new(1, 5 * GST_SECOND, 30 * GST_SECOND);

    else
        sit = gst_mpegts_scte_splice_in_new(2, 35 * GST_SECOND);

    section = gst_mpegts_section_from_scte_sit(sit, 123);
    if (gst_mpegts_section_send_event(section, mux) == TRUE)
    {
        g_print("Enviado\n");
    }
    else
    {
        g_print("Nao Enviado\n");
    }
    gst_mpegts_section_unref(section);
}

static gboolean
send_splice_in(GstElement* mux)
{
    send_splice(mux, FALSE);

    return G_SOURCE_REMOVE;
}

static gboolean
send_splice_out(GstElement* mux)
{
    send_splice(mux, TRUE);

    /* In 30s send the splice-in one */
    g_timeout_add_seconds(15, (GSourceFunc)send_splice_in, mux);

    return G_SOURCE_REMOVE;
}



static gboolean send_scte35(GstElement* muxer) {
    GstBus* bus;
    GstMessage* msg;
    GstMpegtsSection* section;
    GstMpegtsSCTESIT* sit;
    GstMpegtsSCTESpliceEvent* event;
    guint16 i = 500;

    g_print("Enviando SCTE35\n");

    // Crie a seção SCTE-35
    sit = gst_mpegts_scte_sit_new();
    //event = gst_mpegts_scte_splice_event_new(1234, GST_CLOCK_TIME_NONE, 0);
    event = gst_mpegts_scte_sit_new(1, GST_CLOCK_TIME_NONE);
    g_ptr_array_add(sit->splices, event);
    
    section = gst_mpegts_section_from_scte_sit(sit, i);
    gst_mpegts_section_send_event(section, muxer);
    
    g_print("Tipo de secao: %d\n", sit->splice_command_type);
    g_print("Tamanho da secao: %d\n", sit->splice_command_length);

    g_print("Tipo de secao: %d\n", GST_MPEGTS_SECTION_TYPE(section));
    g_print("Tamanho da secao: %d\n", section->section_length);

    // Envie a seção SCTE-35 para o muxer
    msg = gst_message_new_element(GST_OBJECT(muxer),
        gst_structure_new("mpegts-section",
            "section", GST_TYPE_MPEGTS_SECTION, section,
            NULL));
    bus = gst_element_get_bus(muxer);
    gst_bus_post(bus, msg);
    gst_object_unref(bus);
    gst_mpegts_section_unref(section);

    return FALSE; // Retorne FALSE para que a função não seja chamada novamente
}

int main(int argc, char* argv[]) {
    GstElement* pipeline, * source, * filter, * convert, * encoder, * muxer, * sink;
    GstCaps* caps;
    GstBus* bus;
    GstMessage* msg;
    GMainLoop* loop;

    gst_init(&argc, &argv);
    gst_mpegts_initialize();

    pipeline = gst_pipeline_new("meu_pipeline");

    source = gst_element_factory_make("audiotestsrc", "source");
    filter = gst_element_factory_make("capsfilter", "filter");
    convert = gst_element_factory_make("audioconvert", "convert"); // Alterado para audioconvert
    encoder = gst_element_factory_make("lamemp3enc", "encoder"); // Alterado para lamemp3enc
    muxer = gst_element_factory_make("mpegtsmux", "muxer");
    sink = gst_element_factory_make("srtsink", "sink");

    if (!pipeline || !source || !filter || !convert || !encoder || !muxer || !sink) {
        g_printerr("Não foi possível criar um ou mais elementos.\n");
        return -1;
    }

    caps = gst_caps_new_simple("audio/x-raw",
        "format", G_TYPE_STRING, "S16LE",
        "channels", G_TYPE_INT, 2,
        "rate", G_TYPE_INT, 44100,
        NULL);
    g_object_set(G_OBJECT(filter), "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(G_OBJECT(sink), "uri", "srt://:8888", NULL);

    //SCTE 35 ENABLE
    //g_object_set(muxer, "send-scte35-events", TRUE, NULL);
    g_object_set(muxer, "scte-35-pid", 500, NULL);
    g_object_set(muxer, "scte-35-null-interval", 450000000, NULL);


    gst_bin_add_many(GST_BIN(pipeline), source, filter, convert, encoder, muxer, sink, NULL);
    if (!gst_element_link_many(source, filter, convert, encoder, muxer, sink, NULL)) {
        g_printerr("Os elementos não puderam ser vinculados.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    g_print("Iniciado\n");
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_print("Setado PLAYING\n");

    for (int i = 1; i <= 10; i++)
    {
        g_timeout_add_seconds(10 * i, (GSourceFunc)send_splice_out, muxer);
        g_print("SCTE35 agendado %d\n", i);
    }

    //while (1)
    //{
    //    g_usleep(5000000);
    //    send_scte35(muxer);
    //    g_print("SCTE35 Enviado\n");

    //}
    // Entre no loop principal
    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    // Limpeza
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}