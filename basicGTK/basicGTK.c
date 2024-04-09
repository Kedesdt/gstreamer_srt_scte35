#include <gtk/gtk.h>
#include <gst/gst.h>
#include <stdio.h>


// Função para listar as portas seriais
void listar_portas_seriais(GtkComboBoxText* combobox);

// Função para listar os dispositivos de áudio
void listar_dispositivos_audio(GtkComboBoxText* combobox);

void listar_backends_audio(GtkComboBoxText* combobox);

int main(int argc, char* argv[]) {
    GtkWidget* window;
    GtkWidget* combobox_portas, * combobox_audio, * combobox_backends;
    GtkWidget* vbox;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Selecionar Porta e Áudio");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    combobox_backends = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(vbox), combobox_backends, TRUE, TRUE, 0);
    listar_backends_audio(combobox_backends);

    combobox_portas = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(vbox), combobox_portas, TRUE, TRUE, 0);
    listar_portas_seriais(GTK_COMBO_BOX_TEXT(combobox_portas));

    combobox_audio = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(vbox), combobox_audio, TRUE, TRUE, 0);
    listar_dispositivos_audio(GTK_COMBO_BOX_TEXT(combobox_audio));

    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}

void listar_dispositivos_audio(GtkComboBoxText* combobox) {
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

void listar_portas_seriais(GtkComboBoxText* combobox) {
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