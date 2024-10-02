#pragma once
#ifdef __cplusplus
extern "C" {
#endif //Para funcionar em c++
    void listar_dispositivos_audio(char* [], int, const char*, int);
    void listar_dispositivos_audio_s(char* [], int, const char*, int);
    char* obter_id_dispositivo(char*, const char*);
    char* obter_id_dispositivo_s(char*, const char*);

#ifdef __cplusplus
}
#endif //Para funcionar em c++
