#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 16

int main(int argc, char** argv)
{
    int rango, num_procesos;

    // TODO: Inicializar el entorno MPI

    // TODO: Obtener el identificador del proceso actual

    // TODO: Obtener el número total de procesos

    if (N % num_procesos != 0) {
        if (rango == 0) {
            printf("N debe ser divisible por el número de procesos\n");
        }
        // TODO: Finalizar MPI
        return -1;
    }

    int elementos_por_proceso = N / num_procesos;

    int* datos = NULL;
    int* datos_locales = (int*)malloc(elementos_por_proceso * sizeof(int));

    if (rango == 0) {
        datos = (int*)malloc(N * sizeof(int));

        for (int i = 0; i < N; i++) {
            datos[i] = i + 1;
        }
    }

    // TODO: Repartir los datos entre los procesos

    int suma_local = 0;
    for (int i = 0; i < elementos_por_proceso; i++) {
        suma_local += datos_locales[i];
    }

    int suma_global = 0;

    // TODO: Reducir todas las sumas locales en suma_global en el proceso raíz

    if (rango == 0) {
        printf("Suma global = %d\n", suma_global);
    }

 

    free(datos_locales);
    if (rango == 0) {
        free(datos);
    }

    // TODO: Finalizar MPI

    return 0;
}