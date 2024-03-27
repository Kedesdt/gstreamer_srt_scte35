#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <windows.h>
#include <stdio.h>
#include "../util/mySerial.h"

#define COM L"COM6"
#define URI_STR "srt://10.13.24.80:8888"

/*
    Pipeline:
    gst-launch-1.0 -v srtsrc uri=srt://127.0.0.1:8888 ! tsdemux ! mpegaudioparse ! mpg123audiodec ! audioconvert ! autoaudiosink
*/

struct MySerial* serial;


void iterateSplices(GstMpegtsSCTESIT* sit) {
    GPtrArray* splices = sit->splices;
    guint numSplices = splices->len;

    for (guint i = 0; i < numSplices; ++i) {
        GstMpegtsSCTESpliceEvent* spliceEvent = g_ptr_array_index(splices, i);

        
        if (spliceEvent->splice_event_id == 1)
        {   
            g_print("Local\n");
            setDtr(serial, 1);
            g_usleep(500000);
            setDtr(serial, 0);
        }
        else if (spliceEvent->splice_event_id == 2)
        {
            g_print("Rede\n");
            setRts(serial, 1);
            g_usleep(500000);
            setRts(serial, 0);
        }
    }
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

        }
    }
    default:
        ;
    }

    return TRUE;
}

int main(int argc, char* argv[]) {


    LPCWSTR port[10];
    printf("Digite a porta COM: ");
    scanf_s("%9ls", port);

    char uri[100];
    printf("Digite a uri Ex: srt://10.13.24.80:8888 : ");
    scanf_s("%99s", uri);

    g_print("URI %s\n", uri);
    g_print("Porta %ls\n", port);

    serial = (struct MySerial*)malloc(sizeof(struct MySerial));

    init(serial, port);
    setRts(serial, FALSE);
    setDtr(serial, FALSE);
    g_print("Serial Iniciado\n");


    GstElement* pipeline, * demux, * source, * mp3Parse, * audioDecoder, * audioConvert, * sink;
    GstBus* bus;
    GstMessage* msg;
    GstStateChangeReturn ret;

    gst_init(&argc, &argv);

    pipeline = gst_pipeline_new("scte35-pipeline");
    source = gst_element_factory_make("srtclientsrc", "source");
    demux = gst_element_factory_make("tsdemux", "demux");
    mp3Parse = gst_element_factory_make("mpegaudioparse", "mp3Parse");
    audioDecoder = gst_element_factory_make("mpg123audiodec", "audioDecoder");
    audioConvert = gst_element_factory_make("audioconvert", "audioConvert");
    sink = gst_element_factory_make("autoaudiosink", "audioSink");

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, NULL);
    gst_object_unref(bus);


    if (!pipeline || !source || !demux || !mp3Parse || !audioDecoder || !audioConvert || !sink) {
        g_printerr("Não foi possível criar um ou mais elementos.\n");
        return -1;
    }

    //Authentication SRT
    g_object_set(G_OBJECT(source), "authentication", TRUE, NULL);
    g_object_set(G_OBJECT(source), "passphrase", "ExGA8.w8-@", NULL);

    g_object_set(G_OBJECT(source), "uri", uri, NULL);
    g_object_set(G_OBJECT(demux), "send-scte35-events", TRUE, NULL);
    g_object_set(G_OBJECT(demux), "emit-stats", TRUE, NULL);

    gst_bin_add_many(GST_BIN(pipeline), source, demux, mp3Parse, audioDecoder, audioConvert, sink, NULL);
    g_print("source, demux: %d\n", gst_element_link(source, demux));
    g_print("mp3Parse, audioDecoder: %d\n", gst_element_link(mp3Parse, audioDecoder));
    g_print("audioDecoder, audioConvert: %d\n", gst_element_link(audioDecoder, audioConvert));
    g_print("audioConvert, sink: %d\n", gst_element_link(audioConvert, sink));
    //g_print("audioDecoder, sink: %d\n", gst_element_link(audioDecoder, sink));

    g_signal_connect(demux, "pad-added", G_CALLBACK(on_pad_added), mp3Parse);

    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Não foi possível mudar para o estado PLAYING.\n");
        gst_object_unref(pipeline);
        return -1;
    }
    else if (ret == GST_STATE_CHANGE_ASYNC) {
        // Aguarde até que a mudança de estado seja concluída
        GstBus* bus = gst_element_get_bus(pipeline);
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ASYNC_DONE | GST_MESSAGE_ERROR);

        if (msg != NULL) {
            if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
                GError* err;
                gchar* debug_info;

                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Erro recebido do elemento %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Informação de depuração: %s\n", debug_info ? debug_info : "nenhuma");
                g_clear_error(&err);
                g_free(debug_info);

                gst_message_unref(msg);
                gst_object_unref(bus);
                gst_object_unref(pipeline);
                return -1;
            }

            else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ASYNC_DONE) {
                g_print("Mudança de estado para PLAYING concluída.\n");
            }
            gst_message_unref(msg);
        }
        gst_object_unref(bus);
    }

    GMainLoop* loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    // Limpeza
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    destroy(serial);

    return 0;
}