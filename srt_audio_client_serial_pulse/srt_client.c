#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <windows.h>
//#include "serial.h"

#define COM L"COM6"
#define URI_STR "srt://10.13.24.80:8888"

/*
    Pipeline:
    gst-launch-1.0 -v srtsrc uri=srt://127.0.0.1:8888 ! tsdemux ! mpegaudioparse ! mpg123audiodec ! audioconvert ! autoaudiosink
*/


HANDLE hComm;
DCB dcb;
boolean dtr, rts;

struct MySerial serial;


void serialInit()
{
    dtr = 0;
    rts = 0;

    hComm = CreateFile(COM, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (hComm == INVALID_HANDLE_VALUE) {

        g_print("Erro ao abrir a porta serial %s.\n", COM);
        g_print("%s", GetLastError());
        return 1;
    }

    // Configure a porta serial
    SecureZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(hComm, &dcb)) {
        g_print("Erro ao obter o estado da porta serial.\n");
        CloseHandle(hComm);
        return 1;
    }
}

void setDtr(boolean n) {
    
    if (n) 
    {
     
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        if (!SetCommState(hComm, &dcb)) {
            g_print("Erro ao configurar a porta serial.\n");
            CloseHandle(hComm);
            return 1;
        }
    }
    else
    {

        dcb.fDtrControl = DTR_CONTROL_DISABLE;
        if (!SetCommState(hComm, &dcb)) {
            g_print("Erro ao configurar a porta serial.\n");
            CloseHandle(hComm);
            return 1;
        }

    }
    
    
}

void setRts(boolean n) {

    if (n)
    {

        dcb.fRtsControl = RTS_CONTROL_ENABLE;
        if (!SetCommState(hComm, &dcb)) {
            g_print("Erro ao configurar a porta serial.\n");
            CloseHandle(hComm);
            return 1;
        }
    }
    else
    {

        dcb.fRtsControl = RTS_CONTROL_DISABLE;
        if (!SetCommState(hComm, &dcb)) {
            g_print("Erro ao configurar a porta serial.\n");
            CloseHandle(hComm);
            return 1;
        }

    }


}


void iterateSplices(GstMpegtsSCTESIT* sit) {
    GPtrArray* splices = sit->splices;
    guint numSplices = splices->len;

    for (guint i = 0; i < numSplices; ++i) {
        GstMpegtsSCTESpliceEvent* spliceEvent = g_ptr_array_index(splices, i);

        // Acesse os campos relevantes do spliceEvent
        guint32 spliceEventId = spliceEvent->splice_event_id;
        guint64 programSpliceTime = spliceEvent->program_splice_time;
        guint32 utcSpliceTime = spliceEvent->utc_splice_time;

        // Faça algo com os dados do spliceEvent (por exemplo, imprimir)
        g_print("Splice Event ID: %u\n", spliceEventId);
        g_print("Program Splice Time: %" G_GUINT64_FORMAT "\n", programSpliceTime);
        g_print("UTC Splice Time: %u\n", utcSpliceTime);

        g_print("%d ", spliceEvent->insert_event);
        g_print("%d ", spliceEvent->splice_event_id);
        g_print("%d\n ", spliceEvent->splice_event_cancel_indicator);

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

        if (spliceEvent->splice_event_id == 1)
        {
            setDtr(1);
            g_usleep(500000);
            setDtr(0);
        }
        else if (spliceEvent->splice_event_id == 2)
        {
            setRts(1);
            g_usleep(500000);
            setRts(0);
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


    serialInit();
    setDtr(0);
    setRts(0);


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

    g_object_set(G_OBJECT(source), "uri", URI_STR, NULL);
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

    CloseHandle(hComm);

    return 0;
}