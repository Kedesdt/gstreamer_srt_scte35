#include <gtk/gtk.h>
#include <gst/gst.h>
#include <stdio.h>
#include <glib.h>



//void listar_portas_seriais(GtkComboBoxText* combobox);
//void listar_dispositivos_audio(GtkComboBoxText* combobox);
//void listar_dispositivos_audio(GtkComboBoxText* combobox, char* backend);
void on_button_clicked(GtkWidget*, gpointer);
//void listar_backends_audio(GtkComboBoxText* combobox);

int main(int argc, char* argv[]) {
    
    GtkWidget* window;
    GtkWidget* dropdown_portas, * dropdown_audio, * dropdown_backends, * button;
    GtkWidget* vbox;
    GListStore* store;
    GtkDropDown* dropdown;

    gtk_init(&argc, &argv);

    window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "Selecionar Porta e Áudio");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    store = g_list_store_new(G_TYPE_STRING);
    dropdown = GTK_DROP_DOWN(gtk_drop_down_new_from_model(G_LIST_MODEL(store)));
    gtk_box_append(GTK_BOX(vbox), GTK_WIDGET(dropdown));
    //listar_backends_audio(dropdown);

    // Repita para os outros dropdowns...

    button = gtk_button_new_with_label("Start");
    g_signal_connect(GTK_BUTTON(button), "clicked", G_CALLBACK(on_button_clicked), NULL);
    gtk_box_append(GTK_BOX(vbox), button);

    gtk_window_present(GTK_WINDOW(window));

    gtk_main();
    return 0;
}
/*
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
/*void listar_dispositivos_audio(GtkComboBoxText* combobox, gchar* backend) {
    
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
}*

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
}*/

void on_button_clicked(GtkWidget* widget, gpointer data) {
    g_print("Botão clicado!\n");
}

