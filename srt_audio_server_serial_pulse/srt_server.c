#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>
#include <windows.h>
#include "mySerial.h"

/* Pipeline que funciona AAC:
gst-launch-1.0 -v audiotestsrc wave=sine do-timestamp=true ! audioconvert ! audioresample ! "audio/x-raw,format=S16LE,channels=2,rate=48000" ! voaacenc ! aacparse ! queue ! mpegtsmux name=mux ! srtserversink uri=srt://:8888

    Pipeline que funciona MP3:
    gst-launch-1.0 -v audiotestsrc wave=sine do-timestamp=true ! audioconvert ! audioresample ! "audio/x-raw,format=S16LE,channels=2,rate=48000" ! lamemp3enc ! mpegaudioparse ! queue ! mpegtsmux name=mux ! srtserversink uri=srt://:8888


*/

struct MySerial* serial;
GstElement* volume;

static void
send_splice(GstElement* mux, gboolean out)
{
    GstMpegtsSCTESIT* sit;
    GstMpegtsSection* section;

    g_print("Sending Splice %s event\n", out ? "Out" : "In");

    /* Splice is at 5s for 30s */
    if (out) {
        sit = gst_mpegts_scte_splice_out_new(1, 5 * GST_SECOND, 30 * GST_SECOND);
        g_object_set(G_OBJECT(volume), "mute", TRUE, NULL);
    }
    else {
        sit = gst_mpegts_scte_splice_in_new(2, 35 * GST_SECOND);
        g_object_set(G_OBJECT(volume), "mute", FALSE, NULL);
    }

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
    g_object_set(G_OBJECT(volume), "mute", FALSE, NULL);

    return G_SOURCE_REMOVE;
}

static gboolean
send_splice_out(GstElement* mux)
{
    send_splice(mux, TRUE);
    g_object_set(G_OBJECT(volume), "mute", TRUE, NULL);
    

    return G_SOURCE_REMOVE;
}

int main(int argc, char* argv[]) {


    g_print("Iniciando\n");

    serial = (struct MySerial*)malloc(sizeof(struct MySerial));

    init(serial, L"COM7");
    setRts(serial, TRUE);
    g_print("Serial Iniciado\n");

    GstElement* pipeline, *audioSource, * audioFilter, *audioFilter2, * audioConvert, *audioResample, * audioEncoder, * mp3Parse, * audioQueue,
                          * muxer,       * sink;
    GstCaps* caps;
    GstBus* bus;
    GstMessage* msg;
    GstStateChangeReturn ret;
    GMainLoop* loop;

    gst_init(&argc, &argv);
    gst_mpegts_initialize();
    gst_debug_set_default_threshold(GST_LEVEL_ERROR);

    pipeline = gst_pipeline_new("meu_pipeline");

    audioSource   = gst_element_factory_make("wasapisrc",      "audioSource"  );
    audioFilter   = gst_element_factory_make("capsfilter",     "audioFilter"  );
    volume        = gst_element_factory_make("volume",         "volume"       );
    audioConvert  = gst_element_factory_make("audioconvert",   "audioConvert" );
    audioResample = gst_element_factory_make("audioresample",  "audioresample");
    audioFilter2  = gst_element_factory_make("capsfilter",     "audioFilter2" );
    audioEncoder  = gst_element_factory_make("lamemp3enc",     "audioEncoder" );
    mp3Parse      = gst_element_factory_make("mpegaudioparse", "mpegParser"   );
    audioQueue    = gst_element_factory_make("queue",          "audioQueue"   );

    muxer         = gst_element_factory_make("mpegtsmux",      "muxer"        );
    sink          = gst_element_factory_make("srtsink",        "sink"         );

    if (!pipeline || !audioSource || !audioFilter || !audioFilter2 || !volume || !audioConvert || !audioResample || !audioEncoder || !mp3Parse || !audioQueue
                  || !mp3Parse    || !muxer       || !sink)
    {
        g_printerr("Não foi possível criar um ou mais elementos.\n");
        return -1;
    }

    caps = gst_caps_new_simple("audio/x-raw",
        "format",   G_TYPE_STRING, "F32LE",
        "channels", G_TYPE_INT,    2,
        "rate",     G_TYPE_INT,    44100,
        "layout",   G_TYPE_STRING, "interleaved",
        NULL);

    g_object_set(G_OBJECT(audioFilter), "caps", caps, NULL);
    gst_caps_unref(caps);

    caps = gst_caps_new_simple("audio/x-raw",
        "rate", G_TYPE_INT, 48000,
        NULL);

    g_object_set(G_OBJECT(audioFilter2), "caps", caps, NULL);
    gst_caps_unref(caps);


    g_object_set(G_OBJECT(sink), "uri", "srt://:8888", NULL);

   
    g_object_set(G_OBJECT(audioSource), "device", "\{0.0.1.00000000\}.\{eb8a86c3-c59a-4129-ac6a-fa4886311551\}", NULL);
    //g_object_set(G_OBJECT(audioSource), "index", 0, NULL);

    //SCTE 35 ENABLE
    g_object_set(muxer, "scte-35-pid",           500,       NULL);
    g_object_set(muxer, "scte-35-null-interval", 450000000, NULL);

    const gchar* device_value;

    g_object_get(audioSource, "device", &device_value,  NULL);
    g_print("Valor da propriedade \"device\": %s\n", device_value);

    gst_bin_add_many(GST_BIN(pipeline), audioSource, audioFilter, audioFilter2, volume, audioResample, audioEncoder, audioConvert, mp3Parse, audioQueue,
                                        muxer,       sink,        NULL);


    g_print("audioSource,    audioFilter  : %d\n", gst_element_link(audioSource,   audioFilter  ));
    g_print("audioFilter,    audioConvert : %d\n", gst_element_link(audioFilter,   audioConvert ));
    g_print("audioConvert,   volume       : %d\n", gst_element_link(audioConvert,  volume       ));
    g_print("volume,         audioResample: %d\n", gst_element_link(volume,        audioResample));
    g_print("audioRessample, audioFilter2 : %d\n", gst_element_link(audioResample, audioFilter2 ));
    g_print("audioFilter2,   audioEncoder : %d\n", gst_element_link(audioFilter2,  audioEncoder ));
    g_print("audioEncoder,   mp3Parse     : %d\n", gst_element_link(audioEncoder,  mp3Parse     ));
    g_print("mp3Parse,       audioQueue   : %d\n", gst_element_link(mp3Parse,      audioQueue   ));
    g_print("audioQueue,     muxer        : %d\n", gst_element_link(audioQueue,    muxer        ));

    g_print("muxer,          sink         : %d\n", gst_element_link(muxer,         sink         ));

    g_print("Iniciado\n");


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

    g_print("Setado PLAYING\n");

    serial_loop(send_splice, serial, muxer);

    while (1)
    {
        g_usleep(1 * 60 * 1000000);
        //send_splice_out(muxer, volume);
        g_usleep(1 * 60 * 1000000);
    }

    // Limpeza
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;


}