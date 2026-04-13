#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Operación costosa por elemento: por cada par (a[i], b[i]) se realizan
 * múltiples iteraciones con sin(), sqrt() y log() para generar carga
 * de CPU significativa. Así se nota la diferencia al usar más workers.
 */
#define ITERS_PER_ELEMENT 50

static void init_vectors(double *a, double *b, int n)
{
    for (int i = 0; i < n; i++) {
        a[i] = (double)(i + 1) * 0.01;
        b[i] = (double)(i + 1) * 0.001;
    }
}

static double heavy_compute(double x, double y)
{
    double result = x + y;
    for (int k = 0; k < ITERS_PER_ELEMENT; k++) {
        result = sin(result) * sin(result) + cos(result) * cos(result)
               + sqrt(fabs(result + 1.0))
               + log(fabs(result) + 1.0)
               + atan(result);
    }
    return result;
}

static double compute_expected(double x, double y)
{
    return heavy_compute(x, y);
}

static int validate_result(const double *a, const double *b, const double *c, int n)
{
    int errors = 0;
    for (int i = 0; i < n; i++) {
        double expected = compute_expected(a[i], b[i]);
        if (fabs(c[i] - expected) > 1e-9) {
            if (errors < 5) {
                fprintf(stderr, "  Mismatch en i=%d: esperado=%.12f, obtenido=%.12f\n",
                        i, expected, c[i]);
            }
            errors++;
        }
    }
    return errors == 0;
}

int main(int argc, char **argv)
{
    printf("Iniciando programa MPI (cómputo intensivo)...\n");
    int rank, size;
    int n;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            fprintf(stderr, "Uso: %s <tamaño_vector>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    n = atoi(argv[1]);

    printf("[rank %d] n=%d, iteraciones por elemento=%d\n", rank, n, ITERS_PER_ELEMENT);

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

    double *local_a = (double *)malloc((size_t)local_n * sizeof(double));
    double *local_b = (double *)malloc((size_t)local_n * sizeof(double));
    double *local_c = (double *)malloc((size_t)local_n * sizeof(double));

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

    double *a = NULL;
    double *b = NULL;
    double *c = NULL;

    if (rank == 0) {
        a = (double *)malloc((size_t)n * sizeof(double));
        b = (double *)malloc((size_t)n * sizeof(double));
        c = (double *)malloc((size_t)n * sizeof(double));

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
        printf("[root] Operación: %d iteraciones de sin/cos/sqrt/log/atan por elemento\n", ITERS_PER_ELEMENT);
    }

    /* --- Medición de tiempo --- */
    MPI_Barrier(MPI_COMM_WORLD);
    double t_total_start = MPI_Wtime();
    double t_scatter_start = MPI_Wtime();

    MPI_Scatterv(a, send_counts, displs, MPI_DOUBLE, local_a, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatterv(b, send_counts, displs, MPI_DOUBLE, local_b, local_n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double t_scatter_end = MPI_Wtime();

    printf("[rank %d] Procesando %d elementos (offset %d)\n", rank, local_n, displs[rank]);

    double t_compute_start = MPI_Wtime();
    for (int i = 0; i < local_n; i++) {
        local_c[i] = heavy_compute(local_a[i], local_b[i]);
    }
    double t_compute_end = MPI_Wtime();

    printf("[rank %d] Cómputo local: %.6f s\n", rank, t_compute_end - t_compute_start);

    double t_gather_start = MPI_Wtime();
    MPI_Gatherv(local_c, local_n, MPI_DOUBLE, c, send_counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    double t_gather_end = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);
    double t_total_end = MPI_Wtime();

    if (rank == 0) {
        //int ok = validate_result(a, b, c, n);

        //printf("\n[root] Validación: %s\n", ok ? "OK (resultado correcto)" : "ERROR (resultado incorrecto)");
        printf("\n--- Tiempos MPI (rank 0) ---\n");
        printf("  Scatter (distribución):  %.6f s\n", t_scatter_end - t_scatter_start);
        printf("  Cómputo local:           %.6f s\n", t_compute_end - t_compute_start);
        printf("  Gather (recolección):    %.6f s\n", t_gather_end - t_gather_start);
        printf("  Tiempo total:            %.6f s\n", t_total_end - t_total_start);
        printf("----------------------------\n");

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
