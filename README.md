# Suma de vectores en paralelo con MPI (C)

Ejemplo mínimo y didáctico de suma de dos vectores usando **MPI** en C.

El programa:
- Inicializa dos vectores `A` y `B` en el proceso root (rank 0).
- Distribuye segmentos de los vectores entre procesos con `MPI_Scatterv`.
- Suma en paralelo (`C_local = A_local + B_local`).
- Recolecta el resultado final con `MPI_Gatherv`.
- Imprime una muestra del resultado y valida que `C[i] = A[i] + B[i]`.

> Manejo de tamaños no divisibles: **sí está soportado**.
> Se reparte el resto (`N % num_procesos`) entre los primeros procesos.

## Estructura del proyecto

```text
.
├── src/main.c
├── Makefile
├── Dockerfile
├── docker-compose.yml
└── README.md
```

## Requisitos

### Opción local
- `gcc`/`mpicc`
- `OpenMPI`
- `make`

En Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y build-essential make openmpi-bin libopenmpi-dev
```

### Opción contenedor
- Docker
- (Opcional) Docker Compose

## Compilar (local)

```bash
make build
```

Genera el ejecutable en:

```text
bin/vector_sum_mpi
```

## Ejecutar con `mpirun` (local)

Ejemplo con 4 procesos y vector de tamaño 17:

```bash
make run-mpi NP=4 N=17
```

También puedes ejecutar directamente:

```bash
mpirun --allow-run-as-root -np 4 ./bin/vector_sum_mpi 17
```

## Ejecutar sin MPI (1 proceso)

```bash
make run N=17
```

## Ejecutar con Docker

### Construir imagen

```bash
make docker-build
```

### Ejecutar ejemplo en contenedor

```bash
make docker-run NP=4 N=17
```

También directo con Docker:

```bash
docker run --rm vector_sum_mpi:latest mpirun --allow-run-as-root -np 4 /app/bin/vector_sum_mpi 17
```

## Ejecutar con Docker Compose

Construir y ejecutar:

```bash
docker compose up --build
```

Si ya está construida la imagen:

```bash
docker compose up
```

## Usar Make

Objetivos disponibles:

- `make build`      → compila el programa MPI.
- `make run`        → ejecuta en un solo proceso.
- `make run-mpi`    → ejecuta con `mpirun`.
- `make docker-build` → construye la imagen Docker.
- `make docker-run`   → ejecuta el ejemplo dentro de Docker.
- `make clean`      → elimina artefactos (`bin/`).

Variables útiles:
- `NP` = número de procesos MPI (default: `4`)
- `N` = tamaño del vector (default: `17`)

Ejemplo:

```bash
make run-mpi NP=8 N=1000
```

## Limpieza

```bash
make clean
```
