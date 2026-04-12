#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

static void init_vectors(int *a, int *b, int n)
{
    for (int i = 0; i < n; i++) {
        a[i] = i + 1;
        b[i] = (i + 1) * 10;
    }
}

static int validate_result(const int *a, const int *b, const int *c, int n)
{
    for (int i = 0; i < n; i++) {
        if (c[i] != a[i] + b[i]) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv)
{
    printf("Iniciando programa MPI...\n");
    int rank, size;
    int n = 500000; 

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc > 1) {
        n = atoi(argv[1]);
    }

    printf("[rank %d] Argumento n: %d\n", rank, n);

    if (n <= 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: el tamaño del vector debe ser mayor que 0.\n");
        }
        MPI_Finalize();
        return 1;
    }

    int *send_counts = (int *)malloc((size_t)size * sizeof(int));
    int *displs = (int *)malloc((size_t)size * sizeof(int));

    if (send_counts == NULL || displs == NULL) {
        fprintf(stderr, "Proceso %d: error de memoria para metadata de distribución.\n", rank);
        free(send_counts);
        free(displs);
        MPI_Finalize();
        return 1;
    }

    int base = n / size;
    int rem = n % size;

    for (int i = 0; i < size; i++) {
        send_counts[i] = base + (i < rem ? 1 : 0);
        displs[i] = (i == 0) ? 0 : displs[i - 1] + send_counts[i - 1];
    }

    int local_n = send_counts[rank];

    int *local_a = (int *)malloc((size_t)local_n * sizeof(int));
    int *local_b = (int *)malloc((size_t)local_n * sizeof(int));
    int *local_c = (int *)malloc((size_t)local_n * sizeof(int));

    if (local_a == NULL || local_b == NULL || local_c == NULL) {
        fprintf(stderr, "Proceso %d: error de memoria para vectores locales.\n", rank);
        free(send_counts);
        free(displs);
        free(local_a);
        free(local_b);
        free(local_c);
        MPI_Finalize();
        return 1;
    }

    int *a = NULL;
    int *b = NULL;
    int *c = NULL;

    if (rank == 0) {
        a = (int *)malloc((size_t)n * sizeof(int));
        b = (int *)malloc((size_t)n * sizeof(int));
        c = (int *)malloc((size_t)n * sizeof(int));

        if (a == NULL || b == NULL || c == NULL) {
            fprintf(stderr, "Proceso 0: error de memoria para vectores globales.\n");
            free(send_counts);
            free(displs);
            free(local_a);
            free(local_b);
            free(local_c);
            free(a);
            free(b);
            free(c);
            MPI_Finalize();
            return 1;
        }

        init_vectors(a, b, n);
        printf("[root] Tamaño total del vector: %d\n", n);
        printf("[root] Número de procesos MPI: %d\n", size);
    }

    MPI_Scatterv(a, send_counts, displs, MPI_INT, local_a, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(b, send_counts, displs, MPI_INT, local_b, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    printf("[rank %d] Procesando %d elementos (offset %d)\n", rank, local_n, displs[rank]);

    for (int i = 0; i < local_n; i++) {
        local_c[i] = local_a[i] + local_b[i];
    }

    MPI_Gatherv(local_c, local_n, MPI_INT, c, send_counts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int ok = validate_result(a, b, c, n);

        printf("\n[root] Validación: %s\n", ok ? "OK (resultado correcto)" : "ERROR (resultado incorrecto)");

        free(a);
        free(b);
        free(c);
    }

    free(send_counts);
    free(displs);
    free(local_a);
    free(local_b);
    free(local_c);

    MPI_Finalize();
    return 0;
}
