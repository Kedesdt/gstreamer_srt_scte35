#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
//#include <glib.h>
#include <stdio.h>
#include <windows.h>
#include <jansson.h>
#include "../util/mySerial.h"
#include "../util/util.h"

/* Pipeline que funciona AAC:
gst-launch-1.0 -v audiotestsrc wave=sine do-timestamp=true ! audioconvert ! audioresample ! "audio/x-raw,format=S16LE,channels=2,rate=48000" ! voaacenc ! aacparse ! queue ! mpegtsmux name=mux ! srtserversink uri=srt://:8888

    Pipeline que funciona MP3:
    gst-launch-1.0 -v audiotestsrc wave=sine do-timestamp=true ! audioconvert ! audioresample ! "audio/x-raw,format=S16LE,channels=2,rate=48000" ! lamemp3enc ! mpegaudioparse ! queue ! mpegtsmux name=mux ! srtserversink uri=srt://:8888


*/

struct MySerial* serial;
GstElement* volume;

struct Config {
    LPCWSTR* com;
    char* uri;
    char* device;
};
typedef struct {
    GtkComboBoxText* combobox_portas;
    GtkComboBoxText* combobox_audio;
    GtkComboBoxText* combobox_backends;
} Comboboxes;

static void send_splice(GstElement*, gboolean);
static gboolean send_splice_in(GstElement*);
static gboolean send_splice_out(GstElement*);
static int configuration(struct Config*);
//int start_srt_server(char * uri, char * device, struct Myserial * serial, char * backend);
void on_button_clicked(GtkWidget*, gpointer);
void listar_backends_audio(GtkComboBoxText*);
void listar_portas_seriais(GtkComboBoxText*);
void listar_dispositivos_audio(GtkComboBoxText*);
//void listar_dispositivos_audio(GtkComboBoxText* combobox, char* backend);

int start_srt_server(char * uri, char * device, struct Myserial * serial, char * backend) {

    GstElement* pipeline, * audioSource, * audioFilter, * audioFilter2, * audioConvert, * audioResample, * audioEncoder, * mp3Parse, * audioQueue,
        * muxer, * sink;
    GstCaps* caps;
    GstBus* bus;
    GstMessage* msg;
    GstStateChangeReturn ret;
    GMainLoop* loop;

    pipeline      = gst_pipeline_new("meu_pipeline");

    audioSource   = gst_element_factory_make(g_strconcat(backend, "src"), "audioSource");
    audioFilter   = gst_element_factory_make("capsfilter",     "audioFilter"   );
    volume        = gst_element_factory_make("volume",         "volume"        );
    audioConvert  = gst_element_factory_make("audioconvert",   "audioConvert"  );
    audioResample = gst_element_factory_make("audioresample",  "audioresample" );
    audioFilter2  = gst_element_factory_make("capsfilter",     "audioFilter2"  );
    audioEncoder  = gst_element_factory_make("lamemp3enc",     "audioEncoder"  );
    mp3Parse      = gst_element_factory_make("mpegaudioparse", "mpegParser"    );
    audioQueue    = gst_element_factory_make("queue",          "audioQueue"    );

    muxer         = gst_element_factory_make("mpegtsmux",      "muxer"         );
    sink          = gst_element_factory_make("srtsink",        "sink"          );

    if (!pipeline || !audioSource || !audioFilter || !audioFilter2 || !volume || !audioConvert || !audioResample || !audioEncoder || !mp3Parse || !audioQueue
        || !mp3Parse || !muxer || !sink)
    {
        g_printerr("N�o foi poss�vel criar um ou mais elementos.\n");
        return -1;
    }

    caps = gst_caps_new_simple("audio/x-raw",
        "format", G_TYPE_STRING, "F32LE",
        "channels", G_TYPE_INT, 2,
        "rate", G_TYPE_INT, 44100,
        "layout", G_TYPE_STRING, "interleaved",
        NULL);

    g_object_set(G_OBJECT(audioFilter), "caps", caps, NULL);
    gst_caps_unref(caps);

    caps = gst_caps_new_simple("audio/x-raw",
        "rate", G_TYPE_INT, 48000,
        NULL);

    g_object_set(G_OBJECT(audioFilter2), "caps", caps, NULL);
    gst_caps_unref(caps);


    g_object_set(G_OBJECT(sink), "uri", uri, NULL);
    free(uri);

    //g_object_set(G_OBJECT(audioSource), "device", "\{0.0.1.00000000\}.\{eb8a86c3-c59a-4129-ac6a-fa4886311551\}", NULL);
    //g_object_set(G_OBJECT(audioSource), "device", device, NULL);
    g_object_set(G_OBJECT(audioSource), "device-name", device, NULL);
    free(device);

    //SCTE 35 ENABLE
    g_object_set(muxer, "scte-35-pid", 500, NULL);
    g_object_set(muxer, "scte-35-null-interval", 450000000, NULL);

    const gchar* device_value;

    g_object_get(audioSource, "device", &device_value, NULL);
    g_print("Valor da propriedade \"device\": %s\n", device_value);

    gst_bin_add_many(GST_BIN(pipeline), audioSource, audioFilter, audioFilter2, volume, audioResample, audioEncoder, audioConvert, mp3Parse, audioQueue,
        muxer, sink, NULL);


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
        g_printerr("N�o foi poss�vel mudar para o estado PLAYING.\n");
        gst_object_unref(pipeline);
        return -1;
    }
    else if (ret == GST_STATE_CHANGE_ASYNC) {
        // Aguarde at� que a mudan�a de estado seja conclu�da
        GstBus* bus = gst_element_get_bus(pipeline);
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ASYNC_DONE | GST_MESSAGE_ERROR);

        if (msg != NULL) {
            if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
                GError* err;
                gchar* debug_info;

                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Erro recebido do elemento %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Informa��o de depura��o: %s\n", debug_info ? debug_info : "nenhuma");
                g_clear_error(&err);
                g_free(debug_info);

                gst_message_unref(msg);
                gst_object_unref(bus);
                gst_object_unref(pipeline);
                return -1;
            }

            else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ASYNC_DONE) {
                g_print("Mudan�a de estado para PLAYING conclu�da.\n");
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


int main(int argc, char* argv[]) {
    /*
    g_print("Iniciando\n");
    gst_init(&argc, &argv);
    gst_mpegts_initialize();
    gst_debug_set_default_threshold(GST_LEVEL_ERROR);

    int size = 0, ret;
    char* uri, * device;

    struct Config* config = (struct Config*)malloc(sizeof(struct Config));
    config->com = (LPCWSTR*)malloc(100 * sizeof(WCHAR));
    config->uri = (char*)malloc(100 * sizeof(char));
    config->device = (char*)malloc(100 * sizeof(char));

    if (config->com == NULL || config->uri == NULL || config->device == NULL) {
        g_print("Erro ao alocar mem�ria\n");
        return 1;
    }

    configuration(config);

    for (size; config->uri[size] != '\0'; size++) {
        ;
    }

    uri = (char*)malloc((size + 1) * sizeof(char));

    for (int i = 0; config->uri[i - 1] != '\0'; i++) {
        uri[i] = config->uri[i];
    }

    for (size = 0; config->device[size] != '\0'; size++) {
        ;
    }

    device = (char*)malloc((size + 1) * sizeof(char));

    for (int i = 0; config->device[i - 1] != '\0'; i++) {
        device[i] = config->device[i];
    }

    g_print("URI   : %s\n"
        "DEVICE: %s\n", uri, device);*/

    GtkWidget* window;
    GtkWidget* button;
    //GtkComboBoxText* combobox_portas, * combobox_audio, * combobox_backends;
    GtkWidget* vbox;
    Comboboxes comboboxes;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Selecionar Porta e �udio");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    comboboxes.combobox_backends = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(vbox), comboboxes.combobox_backends, TRUE, TRUE, 0);
    listar_backends_audio(GTK_COMBO_BOX_TEXT(comboboxes.combobox_backends));

    comboboxes.combobox_portas = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(vbox), comboboxes.combobox_portas, TRUE, TRUE, 0);
    listar_portas_seriais(GTK_COMBO_BOX_TEXT(comboboxes.combobox_portas));

    comboboxes.combobox_audio = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(vbox), comboboxes.combobox_audio, TRUE, TRUE, 0);
    listar_dispositivos_audio(GTK_COMBO_BOX_TEXT(comboboxes.combobox_audio));

    button = gtk_button_new_with_label("Start");
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), &comboboxes);
    gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

void on_button_clicked(GtkWidget* widget, gpointer data) {

    g_print("Clicked\n");

    Comboboxes* comboboxes = (Comboboxes*)data;

    char* backend_selecionado = gtk_combo_box_text_get_active_text(comboboxes->combobox_backends);
    char* porta_selecionada = gtk_combo_box_text_get_active_text(comboboxes->combobox_portas);
    char* device_name = gtk_combo_box_text_get_active_text(comboboxes->combobox_audio);
    char* uri;

    LPCWSTR porta_wide = char_to_lpcwstr(porta_selecionada);

    serial = (struct MySerial*)malloc(sizeof(struct MySerial));

    init(serial, porta_wide);

    uri = g_strconcat("srt://:", "50969");

    setRts(serial, TRUE);
    g_print("Serial Iniciado\n");


    start_srt_server(uri, device_name, serial, backend_selecionado);

}


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

static int
configuration(struct Config* config) {

    json_error_t error;
    json_t* root = json_load_file("config.json", 0, &error);
    json_t* com;
    json_t* uri_json;
    json_t* device_path;

    if (!root) {
        fprintf(stderr, "Erro ao ler o arquivo JSON: %s\n", error.text);
        return 1;
    }

    com = json_object_get(root, "COM");
    if (!json_is_string(com)) {
        fprintf(stderr, "COM n�o � uma string\n");
        json_decref(root);
        return 1;
    }

    const char* comString = json_string_value(com);
    int comStringLength = strlen(comString) + 1;
    LPWSTR comLPCWSTR = (LPWSTR*)malloc(comStringLength * sizeof(WCHAR));

    MultiByteToWideChar(CP_ACP, 0, comString, -1, comLPCWSTR, comStringLength);

    config->com = comLPCWSTR;

    uri_json = json_object_get(root, "URI");
    if (!json_is_string(uri_json)) {
        fprintf(stderr, "URI n�o � uma string\n");
        json_decref(root);
        return 1;
    }

    config->uri = json_string_value(uri_json);

    device_path = json_object_get(root, "DEVICE");

    if (!json_is_string(device_path)) {
        fprintf(stderr, "Device n�o � uma string\n");
        json_decref(root);
        return 1;
    }

    config->device = json_string_value(device_path);

    g_print(
        "COM    : %s\n"
        "URI    : %s\n"
        "DEVICE : %s\n", json_string_value(com), config->uri, config->device);

    json_decref(root);
    return 0;
}

void
listar_dispositivos_audio(GtkComboBoxText* combobox) {
    GstDeviceMonitor* monitor;
    GstDevice* device;
    GList* devices, * elem;
    GstStructure* props;
    const gchar* klass, * display_name, * backend;

    gst_init(NULL, NULL);

    monitor = gst_device_monitor_new();
    gst_device_monitor_start(monitor);

    devices = gst_device_monitor_get_devices(monitor);
    for (elem = devices; elem; elem = elem->next) {
        device = GST_DEVICE(elem->data);
        klass = gst_device_get_device_class(device);
        if (g_strrstr(klass, "Audio/Source")) {
            display_name = gst_device_get_display_name(device);
            props = gst_device_get_properties(device);
            backend = gst_structure_get_string(props, "device.api");
            gchar* device_info = g_strdup_printf("%s (%s)", display_name, backend ? backend : "Unknown API");
            gtk_combo_box_text_append_text(combobox, device_info);
            g_free(device_info);
            gst_structure_free(props);
        }
        g_object_unref(device);
    }
    g_list_free(devices);
    g_object_unref(monitor);
}

/*void
listar_dispositivos_audio(GtkComboBoxText* combobox, gchar* backend) {

    GstDeviceMonitor* monitor;
    GstDevice* device;
    GList* devices, * elem;
    GstStructure* props;
    const gchar* klass, * display_name, * backend;

    gst_init(NULL, NULL);

    monitor = gst_device_monitor_new();
    gst_device_monitor_start(monitor);

    devices = gst_device_monitor_get_devices(monitor);
    for (elem = devices; elem; elem = elem->next) {
        device = GST_DEVICE(elem->data);
        klass = gst_device_get_device_class(device);
        if (g_strrstr(klass, "Audio/Source")) {
            display_name = gst_device_get_display_name(device);
            props = gst_device_get_properties(device);
            backend = gst_structure_get_string(props, "device.api");
            gchar* device_info = g_strdup_printf("%s (%s)", display_name, backend ? backend : "Unknown API");
            gtk_combo_box_text_append_text(combobox, device_info);
            g_free(device_info);
            gst_structure_free(props);
        }
        g_object_unref(device);
    }
    g_list_free(devices);
    g_object_unref(monitor);
}*/

void
listar_portas_seriais(GtkComboBoxText* combobox) {
    for (int i = 1; i <= 15; i++) {
        char port_name[10];
        snprintf(port_name, sizeof(port_name), "COM%d", i);
        gtk_combo_box_text_append_text(combobox, port_name);
    }
}

void listar_backends_audio(GtkComboBoxText* combobox) {

    GstDeviceMonitor* monitor;
    GstDevice* device;
    GList* devices, * elem;
    GstStructure* props;
    GHashTable* backends_hash = g_hash_table_new(g_str_hash, g_str_equal);
    gchar* backend;

    gst_init(NULL, NULL);

    monitor = gst_device_monitor_new();
    gst_device_monitor_add_filter(monitor, "Audio/Source", NULL);
    gst_device_monitor_start(monitor);

    devices = gst_device_monitor_get_devices(monitor);
    for (elem = devices; elem; elem = elem->next) {
        device = GST_DEVICE(elem->data);
        props = gst_device_get_properties(device);
        backend = g_strdup(gst_structure_get_string(props, "device.api"));

        if (backend && !g_hash_table_contains(backends_hash, backend)) {
            g_hash_table_insert(backends_hash, backend, backend);
            gtk_combo_box_text_append_text(combobox, backend);
        }
        else {
            g_free(backend);
        }

        gst_structure_free(props);
        g_object_unref(device);
    }
    g_list_free(devices);
    g_object_unref(monitor);
    g_hash_table_destroy(backends_hash);
}

