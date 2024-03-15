#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>

#define GST_USE_UNSTABLE_API

/* Pipeline que funciona AAC:
gst-launch-1.0 -v videotestsrc do-timestamp=true ! "video/x-raw,format=I420,width=640,height=480,framerate=30/1" ! x264enc tune=zerolatency ! video/x-h264,profile=baseline ! queue ! mpegtsmux name=mux ! srtserversink uri=srt://:8888 audiotestsrc wave=sine do-timestamp=true ! audioconvert ! audioresample ! "audio/x-raw,format=S16LE,channels=2,rate=48000" ! voaacenc ! aacparse ! queue ! mux.
    
    Pipeline que funciona MP3:
    gst-launch-1.0 -v videotestsrc do-timestamp=true ! "video/x-raw,format=I420,width=640,height=480,framerate=30/1" ! x264enc tune=zerolatency ! video/x-h264,profile=baseline ! queue ! mpegtsmux name=mux ! srtserversink uri=srt://:8888 audiotestsrc wave=sine do-timestamp=true ! audioconvert ! audioresample ! "audio/x-raw,format=S16LE,channels=2,rate=48000" ! lamemp3enc ! mpegaudioparse ! queue ! mux.


*/

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


int main(int argc, char* argv[]) {
    
    GstElement* pipeline, //* videoSource,  * videoFilter, * videoConvert, * videoEncoder, * videoQueue, 
                          * audioSource,  * audioFilter, * audioConvert, * audioEncoder, * mp3Parse, * audioQueue, 
                          * muxer,        * sink;
    GstCaps* caps;
    GstBus* bus;
    GstMessage* msg;
    GstStateChangeReturn ret;
    GMainLoop* loop;

    gst_init(&argc, &argv);
    gst_mpegts_initialize();

    pipeline = gst_pipeline_new("meu_pipeline");

    //videoSource = gst_element_factory_make("videotestsrc", "videoSource");
    //videoFilter = gst_element_factory_make("capsfilter", "videoFilter");
    //videoConvert = gst_element_factory_make("videoconvert", "videoConvert");
    //videoEncoder = gst_element_factory_make("x264enc", "videoEncoder");
    //videoQueue = gst_element_factory_make("queue", "videoQueue");

    audioSource = gst_element_factory_make("audiotestsrc", "audioSource");
    audioFilter = gst_element_factory_make("capsfilter", "audioFilter");
    audioConvert = gst_element_factory_make("audioconvert", "audioConvert");
    audioEncoder = gst_element_factory_make("lamemp3enc", "audioEncoder");
    mp3Parse = gst_element_factory_make("mpegaudioparse", "mpegParser");
    audioQueue = gst_element_factory_make("queue", "audioQueue");

    muxer = gst_element_factory_make("mpegtsmux", "muxer");
    sink = gst_element_factory_make("srtsink", "sink");

    if (!pipeline //|| !videoSource || !videoFilter || !videoConvert || !videoEncoder || !videoQueue
                  || !audioSource || !audioFilter || !audioConvert || !audioEncoder || !mp3Parse   || !audioQueue
                  || !mp3Parse    || !muxer       || !sink) 
    {
        g_printerr("Não foi possível criar um ou mais elementos.\n");
        return -1;
    }

    //caps = gst_caps_new_simple("video/x-raw",
    //    "format", G_TYPE_STRING, "I420",
    //    "height", G_TYPE_INT, 480,
    //    "width", G_TYPE_INT, 640,
    //    NULL);
    //g_object_set(G_OBJECT(videoFilter), "caps", caps, NULL);
    //gst_caps_unref(caps);

    //g_object_set(G_OBJECT(videoEncoder), "tune", "zerolatency", NULL);

    //caps = gst_caps_new_simple("video/x-h264",
    //    "profile", G_TYPE_STRING, "baseline",
    //    NULL);
    //g_object_set(G_OBJECT(videoEncoder), "caps", caps, NULL);
    //gst_caps_unref(caps);

    //g_object_set(G_OBJECT(videoSource), "do-timestamp", TRUE, NULL);

    caps = gst_caps_new_simple("audio/x-raw",
        "format", G_TYPE_STRING, "S16LE",
        "channels", G_TYPE_INT, 2,
        "rate", G_TYPE_INT, 48000,
        "layout", G_TYPE_STRING, "interleaved", 
        NULL);

    g_object_set(G_OBJECT(audioFilter), "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(G_OBJECT(sink), "uri", "srt://:8888", NULL);

    //SCTE 35 ENABLE
    //g_object_set(muxer, "send-scte35-events", TRUE, NULL);
    g_object_set(muxer, "scte-35-pid", 500, NULL);
    g_object_set(muxer, "scte-35-null-interval", 450000000, NULL);


    gst_bin_add_many(GST_BIN(pipeline), //videoSource, videoFilter, videoConvert, videoEncoder, videoQueue,
                                        audioSource, audioEncoder, audioConvert, audioFilter, mp3Parse, audioQueue, 
                                        muxer, sink, NULL);

    //g_print("videoSource, videoFilter: %d\n", gst_element_link(videoSource, videoFilter));
    //g_print("videoFilter, videoConvert: %d\n", gst_element_link(videoFilter, videoConvert));
    //g_print("videoConvert, videoEncoder: %d\n", gst_element_link(videoConvert, videoEncoder));
    //g_print("videoEncoder, videoQueue: %d\n", gst_element_link(videoEncoder, videoQueue));
    //g_print("videoQueue, muxer: %d\n", gst_element_link(videoQueue, muxer));

    g_print("audioSource, audioFilter: %d\n", gst_element_link(audioSource, audioFilter));
    g_print("audioFilter, audioConvert: %d\n", gst_element_link(audioFilter, audioConvert));
    g_print("audioConvert, audioEncoder: %d\n", gst_element_link(audioConvert, audioEncoder));
    //g_print("audioFilter, audioEncoder: %d\n", gst_element_link(audioFilter, audioEncoder));
    //g_print("audioSource, audioEncoder: %d\n", gst_element_link(audioSource, audioEncoder));
    g_print("audioEncoder, mp3Parse: %d\n", gst_element_link(audioEncoder, mp3Parse));
    g_print("mp3Parse, audioQueue: %d\n", gst_element_link(mp3Parse, audioQueue));
    g_print("audioQueue, muxer: %d\n", gst_element_link(audioQueue, muxer));

    g_print("muxer, sink: %d\n", gst_element_link(muxer, sink));

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

    //g_print("%d\n", ret);
   
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