#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>


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

static GstPadProbeReturn cb_have_data(GstPad* pad,
    GstPadProbeInfo* info,
    gpointer         user_data) {
    GstBuffer* buffer;
    GstMapInfo map;
    GstMpegtsSection* section;

    buffer = GST_PAD_PROBE_INFO_BUFFER(info);

    //inspecionar o buffer para mensagens SCTE-35

    // Mapear o buffer em memória
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    // Criar uma seção a partir dos dados do buffer
    section = gst_mpegts_section_new(0, map.data, map.size);
    
    if (section == NULL)
    {
        g_print("Null");
    }

    g_print("%d ", (*section).pid);
    g_print("%d ", section->table_id);

    g_print("%d ", section->subtable_extension);
    g_print("%d ", section->version_number);

    g_print("%d ", section->current_next_indicator);

    g_print("%d ", section->section_number);
    g_print("%d ", section->last_section_number);

    g_print("%d ", section->crc);

    g_print("%d ", section->data);
    /* section_length: length of data (including final CRC if present) */
    g_print("%d\n", section->section_length);
    /* cached_parsed: cached copy of parsed section */
    //g_print("%d ", section->cached_parsed);
    /* destroy_parsed: function to clear cached_parsed */
    //g_print("%d ", section->destroy_parsed);
    /* offset: offset of the section within the container stream */
    //g_print("%d ", section->offset);
    /* short_section: TRUE if section_syntax_indicator == 0
     * FIXME : Maybe make public later on when allowing creation of
     * sections to that people can create private short sections ? */
    //g_print("%d\n", section->short_section);
    
    //g_print("%d ", section->pid);

    // Verificar se a seção é uma mensagem SCTE-35
    if (GST_MPEGTS_SECTION_TYPE(section) == GST_MPEGTS_SECTION_SCTE_SIT) {
        
        g_print("SCTE-35\n");
        //GstMpegtsSCTE35SpliceInfoSection* splice_info;
        // Parseie a seção SCTE-35
        //splice_info = gst_mpegts_section_get_scte35_splice_info(section);
        //g_print("splice_event_id: %u\n", splice_info->splice_event_id);
        //g_print("splice_event_cancel_indicator: %d\n", splice_info->splice_event_cancel_indicator);
        // Aqui você pode inspecionar o splice_info para as informações SCTE-35
    }
    else
    {
        //g_print("%d ", GST_MPEGTS_SECTION_TYPE(section));
    }


    return GST_PAD_PROBE_OK;
}

int main(int argc, char* argv[]) {
    GstElement* pipeline, * source, * demux, * decodebin, * sink;
    GstPad* pad;

    gst_init(&argc, &argv);

    pipeline = gst_pipeline_new("meu_pipeline");

    source = gst_element_factory_make("srtclientsrc", "source");
    demux = gst_element_factory_make("tsdemux", "demux");
    decodebin = gst_element_factory_make("decodebin", "decodebin");
    sink = gst_element_factory_make("autovideosink", "sink");

    if (!pipeline || !source || !demux || !decodebin || !sink) {
        g_printerr("Não foi possível criar um ou mais elementos.\n");
        return -1;
    }

    g_object_set(G_OBJECT(source), "uri", "srt://127.0.0.1:8888", NULL);

    gst_bin_add_many(GST_BIN(pipeline), source, demux, decodebin, sink, NULL);
    if (!gst_element_link_many(source, demux, NULL)) {
        g_printerr("Os elementos não puderam ser vinculados.\n");
        gst_object_unref(pipeline);
        return -1;
    }
    g_signal_connect(demux, "pad-added", G_CALLBACK(on_pad_added), decodebin);
    g_signal_connect(decodebin, "pad-added", G_CALLBACK(on_pad_added), sink);

    pad = gst_element_get_static_pad(demux, "sink");
    gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
        (GstPadProbeCallback)cb_have_data, NULL, NULL);
    gst_object_unref(pad);

    // Execute o pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Entre no loop principal
    GMainLoop* loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    // Limpeza
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}