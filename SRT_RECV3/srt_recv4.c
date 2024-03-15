#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
//#include <glib.h>


void iterateSplices(GstMpegtsSCTESIT* sit) {
    GPtrArray* splices = sit->splices;
    guint numSplices = splices->len;

    for (guint i = 0; i < numSplices; ++i) {
        GstMpegtsSCTESpliceEvent* spliceEvent = g_ptr_array_index(splices, i);

        // Acesse os campos relevantes do spliceEvent
        guint32 spliceEventId = spliceEvent->splice_event_id;
        guint64 programSpliceTime = spliceEvent->program_splice_time;
        guint32 utcSpliceTime = spliceEvent->utc_splice_time;

        // Fa�a algo com os dados do spliceEvent (por exemplo, imprimir)
        g_print("Splice Event ID: %u\n", spliceEventId);
        g_print("Program Splice Time: %" G_GUINT64_FORMAT "\n", programSpliceTime);
        g_print("UTC Splice Time: %u\n", utcSpliceTime);
        
        g_print("%d ", spliceEvent->insert_event);

        g_print("%d ", spliceEvent->splice_event_id);
        g_print("%d ", spliceEvent->splice_event_cancel_indicator);

        /* If splice_event_cancel_indicator == 0 */
        g_print("%d ", spliceEvent->out_of_network_indicator);
        g_print("%d ", spliceEvent->program_splice_flag);
        g_print("%d ", spliceEvent->duration_flag);

        g_print("%d ", spliceEvent->splice_immediate_flag); /* Only valid for insert_event */

        g_print("%d ", spliceEvent->program_splice_time_specified); /* Only valid for insert_event && program_splice */
        g_print("%d ", spliceEvent->program_splice_time); /* Only valid for insert_event && program_splice */

        g_print("%d ", spliceEvent->utc_splice_time); /* Only valid for !insert_event (schedule) && program_splice */

        g_print("%d ", spliceEvent->components); /* Only valid for !program_splice */

        g_print("%d ", spliceEvent->break_duration_auto_return);
        g_print("%d ", spliceEvent->break_duration);

        g_print("%d ", spliceEvent->unique_program_id);
        g_print("%d ", spliceEvent->avail_num);
        g_print("%d\n", spliceEvent->avails_expected);
    }
}


static void on_scte_35(GstElement* demux, GstMpegtsSection* section, gpointer user_data)
{
    /* Esta fun��o ser� chamada sempre que uma mensagem SCTE-35 for emitida pelo tsparse */
    //g_print(pid);
    g_print("SCTE35");
    /* Aqui voc� pode adicionar seu c�digo para processar a mensagem SCTE-35 */
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

        GstMpegtsSection* section = gst_message_parse_mpegts_section(msg);
        GstMpegtsSCTESIT* sit;
        GstMpegtsSCTESpliceEvent* event;

        
        if (!section)
            break;


        //g_print("%d ", section->section_type);
        if (section->section_type == GST_MPEGTS_SECTION_SCTE_SIT)
        {   
            g_print("Pacote SCTE-35 Chegou\n");

            sit = gst_mpegts_section_get_scte_sit(section);

            iterateSplices(sit);
          

            g_print("Splice_type: %d\n", sit->splice_command_type);

            /*g_print("%d ", section->pid);
            g_print("%d ", section->section_type);

            g_print("%d ", section->table_id);

            g_print("%d ", section->subtable_extension);
            g_print("%d ", section->version_number);

            g_print("%d ", section->current_next_indicator);

            g_print("%d ", section->section_number);
            g_print("%d ", section->last_section_number);

            g_print("%d\n ", section->crc);*/
        }
        //g_print(".");
        const GstStructure* s = gst_message_get_structure(msg);

        //gchar* s_str = gst_structure_to_string(s);
        //g_print("%s\n", s_str);
        //g_free(s_str);

        //g_print(s->name);
        if (gst_structure_has_name(s, "mpegts-section")) {

            g_print("MSG");
            // Esta � uma mensagem SCTE-35
            // Aqui voc� pode adicionar seu c�digo para processar a mensagem SCTE-35
        }
        break;
    }
    default:
        ;
    }

    return TRUE;
}

int main(int argc, char* argv[]) {
    GstElement* pipeline, * demux, * source, * decodebin, * sink;
    GstBus* bus;

    gst_init(&argc, &argv);

    pipeline = gst_pipeline_new("scte35-pipeline");
    demux = gst_element_factory_make("tsdemux", "demux");

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, NULL);
    gst_object_unref(bus);

    source = gst_element_factory_make("srtclientsrc", "source");
    decodebin = gst_element_factory_make("decodebin", "decodebin");
    sink = gst_element_factory_make("autoaudiosink", "sink"); // Alterado para autoaudiosink

    if (!pipeline || !source || !demux || !decodebin || !sink) {
        g_printerr("N�o foi poss�vel criar um ou mais elementos.\n");
        return -1;
    }

    g_object_set(G_OBJECT(source), "uri", "srt://127.0.0.1:8888", NULL);
    g_object_set(G_OBJECT(demux), "send-scte35-events", TRUE, NULL);
    g_object_set(G_OBJECT(demux), "emit-stats", TRUE, NULL);

    gst_bin_add_many(GST_BIN(pipeline), source, demux, decodebin, sink, NULL);
    if (!gst_element_link_many(source, demux, NULL)) {
        g_printerr("Os elementos n�o puderam ser vinculados.\n");
        gst_object_unref(pipeline);
        return -1;
    }
    g_signal_connect(demux, "pad-added", G_CALLBACK(on_pad_added), decodebin);
    g_signal_connect(decodebin, "pad-added", G_CALLBACK(on_pad_added), sink);

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