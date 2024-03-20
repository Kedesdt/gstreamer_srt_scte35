// Arquivo: main.c

#include <stdio.h>
#include "outra.h"

int somaInteiros(int x, int y) {
    return x + y;
}

int main() {
    struct MinhaStruct valores;
    valores.num1 = 10;
    valores.num2 = 20;


    // Chamada da função somaInteiros (membro da estrutura)
    int resultado = valores.somaInteiros(1, 2);
    printf("A soma dos inteiros é: %d\n", valores.num1);

    return 0;
}