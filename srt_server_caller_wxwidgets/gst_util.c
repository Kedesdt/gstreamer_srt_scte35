#include "gst_util.h"
#include <gst/gst.h>


void
listar_dispositivos_audio(char* list[], int size1,const char* api, int size2) {
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

char* obter_id_dispositivo(char* nome_dispositivo,const char* api) {


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
        if (g_strrstr(klass, "Audio/Source")) {
            display_name = gst_device_get_display_name(device);
            props = gst_device_get_properties(device);
            backend = gst_structure_get_string(props, "device.api");

            if (strcmp(backend, api) == 0 && strcmp(display_name, nome_dispositivo) == 0) {
                device_id = gst_structure_get_string(props, "device.strid");
                id_dispositivo = (char*)malloc((strlen(device_id) + 1) * sizeof(char));
                strcpy_s(id_dispositivo, 100, device_id);
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

