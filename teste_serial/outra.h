// Arquivo: minha_struct.h

#ifndef MINHA_STRUCT_H
#define MINHA_STRUCT_H

// Definição da estrutura MinhaStruct
struct MinhaStruct {
    int num1;
    int num2;

    // Função para somar dois inteiros (membro da estrutura)
    int (*somaInteiros)(int, int);
};

#endif