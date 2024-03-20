#include <gst/gst.h>

int main(int argc, char* argv[]) {
    GstElement* pipeline, * directsoundsrc, * audioFilter,  * audioconvert, * autoaudiosink, *volume, *audioresample, *audioEncoder, *audioFilter2, *queue, *mp3Parse, *mp3Parse2, *tsparse, *mpg123audiodec, *muxer , *demux;
    GstBus* bus;
    GstMessage* msg;
    GstStateChangeReturn ret;
    GstCaps *caps;

    // Inicialização do GStreamer
    gst_init(&argc, &argv);

    gst_debug_set_default_threshold(GST_LEVEL_FIXME);

    // Criação dos elementos
    pipeline = gst_pipeline_new("my-pipeline");
    directsoundsrc = gst_element_factory_make("wasapi2src", "directsoundsrc");

    // Configuração do dispositivo DirectSound
    g_object_set(directsoundsrc, "device", "\\\\\?\\SWD\#MMDEVAPI\#\{0.0.1.00000000\}.\{d59148b0-e4e6-4fd1-85f5-5eff3051bee9\}\#\{2eef81be-33fa-4800-9670-1cd474972c3f\}", NULL);

    audioFilter = gst_element_factory_make("capsfilter", "audioFilter");
    audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
    volume = gst_element_factory_make("volume", "volume");
    audioresample = gst_element_factory_make("audioresample", "audioresample");
    queue = gst_element_factory_make("queue", "queue");
    audioFilter2 = gst_element_factory_make("capsfilter", "audioFilter2");
    audioEncoder = gst_element_factory_make("lamemp3enc", "audioEncoder");
    mp3Parse = gst_element_factory_make("mpegaudioparse", "mp3Parser");
    mp3Parse2 = gst_element_factory_make("mpegaudioparse", "mp3Parser2");
    tsparse = gst_element_factory_make("tsparse", "tsparse");
    mpg123audiodec = gst_element_factory_make("mpg123audiodec", "mp3dec");
    demux = gst_element_factory_make("tsdemux", "demux");

    muxer = gst_element_factory_make("mpegtsmux", "muxer");

    autoaudiosink = gst_element_factory_make("autoaudiosink", "autoaudiosink");

    if (!pipeline || !directsoundsrc || !audioconvert || !autoaudiosink || !audioFilter || !volume || !audioEncoder || !audioresample || !queue || !mp3Parse) {
        g_printerr("Não foi possível criar os elementos.\n");
        return -1;
    }

    caps = gst_caps_new_simple("audio/x-raw",
        "format", G_TYPE_STRING, "F32LE",
        "channels", G_TYPE_INT, 16,
        "rate", G_TYPE_INT, 44100,
        "layout", G_TYPE_STRING, "interleaved",
        NULL);

    g_object_set(G_OBJECT(audioFilter), "caps", caps, NULL);
    gst_caps_unref(caps);

    caps = gst_caps_new_simple("audio/x-raw",
        "format", G_TYPE_STRING, "S16LE",
        "channels", G_TYPE_INT, 2,
        "rate", G_TYPE_INT, 44100,
        "layout", G_TYPE_STRING, "interleaved",
        NULL);

    g_object_set(G_OBJECT(audioFilter2), "caps", caps, NULL);
    gst_caps_unref(caps);


    // Montagem da pipeline
    gst_bin_add_many(GST_BIN(pipeline), directsoundsrc, audioFilter, audioconvert, volume, audioresample, audioEncoder, mp3Parse, mpg123audiodec, autoaudiosink, queue, NULL);
    gst_element_link_many(directsoundsrc, audioFilter, volume, audioresample, audioconvert, queue, audioEncoder, mp3Parse, mpg123audiodec, autoaudiosink, NULL);

    // Mudança para o estado PLAYING
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Não foi possível mudar para o estado PLAYING.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Aguarda até que o pipeline termine ou ocorra um erro
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
    if (msg != NULL) {
        gst_message_unref(msg);
    }
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}