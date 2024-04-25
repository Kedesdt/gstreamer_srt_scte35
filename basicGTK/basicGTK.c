#include <gtk/gtk.h>
#include <gst/gst.h>
#include <stdio.h>

#define MAX_ITEMS 10
#define MAX_STR 50


void listar_backends_audio(char**);

static void print_hello(GtkWidget* widget, gpointer data)
{
    g_print("Hello World\n");
}
void modificar_array(char* items[]) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        //printf("..%s\n", items[i]);
        if (items[i] != NULL) {
            strcat_s(items[i], 50, " modificado");
        }
        //printf("%s", items[i]);
    }
}
static void activate(GtkApplication* app, gpointer user_data)
{
    GtkWidget* window;
    GtkWidget* button;
    GtkWidget* dropdown;
    GtkWidget* vbox;

    char** inputs = malloc(MAX_ITEMS * sizeof(char*));

    for (int i = 0; i < MAX_ITEMS; i++) {
        // Aloca memória para cada string
        inputs[i] = malloc(MAX_STR * sizeof(char));
        strcpy_s(inputs[i], MAX_STR, "TESTE");
        printf("%s\n", inputs[i]);
    }

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Hello");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    button = gtk_button_new_with_label("Hello World");
    g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
    gtk_box_append(GTK_BOX(vbox), button);

    listar_backends_audio(inputs);

    dropdown = gtk_drop_down_new_from_strings(inputs);
    gtk_box_append(GTK_BOX(vbox), dropdown);
    gtk_window_present(GTK_WINDOW(window));
}
int main(int    argc,
    char** argv)
{
    GtkApplication* app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

void listar_backends_audio(char** items) {

    GstDeviceMonitor* monitor;
    GstDevice* device;
    GList* devices, * elem;
    GstStructure* props;
    GHashTable* backends_hash = g_hash_table_new(g_str_hash, g_str_equal);
    gchar* backend;
    int index = 0;

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
            printf("%i: %s\n", index, backend);
            strcpy_s(items[index], MAX_STR, backend);
            index++;
        }
        else {
            g_free(backend);
        }

        gst_structure_free(props);
        g_object_unref(device);
    }
    items[index] = NULL;
    g_list_free(devices);
    g_object_unref(monitor);
    g_hash_table_destroy(backends_hash);
}

/*int main() {

    char** inputs = malloc(MAX_ITEMS * sizeof(char*));

    for (int i = 0; i < MAX_ITEMS; i++) {
        // Aloca memória para cada string
        inputs[i] = malloc(MAX_STR * sizeof(char));
        strcpy_s(inputs[i], MAX_STR, "TESTE");
        printf("%s\n", inputs[i]);
    }
    listar_backends_audio(inputs);

    printf("saiu da função");

    for (int i = 0; i < MAX_ITEMS; i++) {
        printf("%s", inputs[i]);
    }

    return 0;
}*/