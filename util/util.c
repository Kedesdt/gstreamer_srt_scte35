#include <windows.h>

LPCWSTR char_to_lpcwstr(const char* char_string) {
    int string_len = MultiByteToWideChar(CP_UTF8, 0, char_string, -1, NULL, 0);
    wchar_t* wide_string = (wchar_t*)malloc(string_len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, char_string, -1, wide_string, string_len);
    return wide_string;
}