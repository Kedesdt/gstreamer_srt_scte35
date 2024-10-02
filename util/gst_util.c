#include "gst_util.h"
#include <gst/gst.h>


void
listar_dispositivos_audio(char*list[], int size1, char* api, int size2) {
    GstDeviceMonitor* monitor;
    GstDevice* device;
    GList* devices, * elem;
    GstStructure* props;
    const gchar* klass, * display_name, * backend;

    gst_init(NULL, NULL);

    monitor = gst_device_monitor_new();
    gst_device_monitor_start(monitor);

    devices = gst_device_monitor_get_devices(monitor);

    int index = 0;

    for (elem = devices; elem; elem = elem->next) {
        device = GST_DEVICE(elem->data);
        klass = gst_device_get_device_class(device);
        if (g_strrstr(klass, "Audio/Source")) {
            display_name = gst_device_get_display_name(device);
            props = gst_device_get_properties(device);
            backend = gst_structure_get_string(props, "device.api");
            if (strcmp(backend, api) == 0) {

                gchar* device_info = g_strdup_printf("%s", display_name);
                list[index] = (char*)malloc(size2 * sizeof(char));

                strcpy_s(list[index], size2, device_info);

                index++;
                g_free(device_info);
            }
            gst_structure_free(props);
        }
        g_object_unref(device);
    }
    g_list_free(devices);
    g_object_unref(monitor);
}
void
listar_dispositivos_audio_s(char* list[], int size1, char* api, int size2) {
    GstDeviceMonitor* monitor;
    GstDevice* device;
    GList* devices, * elem;
    GstStructure* props;
    const gchar* klass, * display_name, * backend;

    gst_init(NULL, NULL);

    monitor = gst_device_monitor_new();
    gst_device_monitor_start(monitor);

    devices = gst_device_monitor_get_devices(monitor);

    int index = 0;

    for (elem = devices; elem; elem = elem->next) {
        device = GST_DEVICE(elem->data);
        klass = gst_device_get_device_class(device);
        printf("%s\n", klass);
        
        if (g_strrstr(klass, "Audio/Sink")) {
            display_name = gst_device_get_display_name(device);
            props = gst_device_get_properties(device);
            backend = gst_structure_get_string(props, "device.api");
            if (strcmp(backend, api) == 0) {

                gchar* device_info = g_strdup_printf("%s", display_name);
                list[index] = (char*)malloc(size2 * sizeof(char));

                strcpy_s(list[index], size2, device_info);

                index++;
                g_free(device_info);
            }
            gst_structure_free(props);
        }
        g_object_unref(device);
    }
    g_list_free(devices);
    g_object_unref(monitor);
}

char* obter_id_dispositivo(char* nome_dispositivo, char* api) {


    GstDeviceMonitor* monitor;
    GstDevice* device;
    GList* devices, * elem;
    GstStructure* props;
    const gchar* klass, * display_name, * backend, * device_id;


    gst_init(NULL, NULL);
    monitor = gst_device_monitor_new();
    gst_device_monitor_start(monitor);

    devices = gst_device_monitor_get_devices(monitor);

    char* id_dispositivo = NULL;

    for (elem = devices; elem; elem = elem->next) {
        device = GST_DEVICE(elem->data);
        klass = gst_device_get_device_class(device);
        gchar* string_devices = g_strrstr(klass, "Audio/Source");
        if (string_devices) {
            display_name = gst_device_get_display_name(device);
            props = gst_device_get_properties(device);
            backend = gst_structure_get_string(props, "device.api");

            if (strcmp(backend, api) == 0 && strcmp(display_name, nome_dispositivo) == 0) {
                device_id = gst_structure_get_string(props, "device.strid");
                id_dispositivo = (char*)malloc((strlen(device_id) + 1) * sizeof(char));
                strcpy_s(id_dispositivo, strlen(device_id) + 1, device_id);
                break;
            }
            gst_structure_free(props);
        }
        g_object_unref(device);
    }
    g_list_free(devices);
    g_object_unref(monitor);

    return id_dispositivo;
}

char* obter_id_dispositivo_s(char* nome_dispositivo, char* api) {

    g_print("1");
    GstDeviceMonitor* monitor;
    GstDevice* device;
    GList* devices, * elem;
    GstStructure* props;
    const gchar* klass, * display_name, * backend, * device_id;


    gst_init(NULL, NULL);
    monitor = gst_device_monitor_new();
    gst_device_monitor_start(monitor);

    devices = gst_device_monitor_get_devices(monitor);

    char* id_dispositivo = NULL;
    g_print("2");
    for (elem = devices; elem; elem = elem->next) {
        device = GST_DEVICE(elem->data);
        klass = gst_device_get_device_class(device);
        gchar* string_devices = g_strrstr(klass, "Audio/Sink");
        g_print("3");
        if (string_devices) {
            display_name = gst_device_get_display_name(device);
            props = gst_device_get_properties(device);
            backend = gst_structure_get_string(props, "device.api");

            if (strcmp(backend, api) == 0 && strcmp(display_name, nome_dispositivo) == 0) {

                g_print("4");
                device_id = gst_structure_get_string(props, "device.strid");
                id_dispositivo = (char*)malloc((strlen(device_id) + 1) * sizeof(char));
                strcpy_s(id_dispositivo, strlen(device_id) + 1, device_id);
                break;
            }
            gst_structure_free(props);
        }
        g_object_unref(device);
    }
    g_list_free(devices);
    g_object_unref(monitor);
    g_print("5 %s", id_dispositivo);

    return id_dispositivo;
}