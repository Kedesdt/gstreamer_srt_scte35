#include <errno.h>
#include <stdio.h>

int main_m() {
    
    int a, b;
    b = 0;

    a = 2 / b;

    printf("Valor de errno: %d\n", errno);
    printf("A mensagem de erro é: %s\n", strerror(errno));
    return 0;
}