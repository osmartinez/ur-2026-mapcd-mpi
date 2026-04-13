# Suma de vectores en paralelo con MPI — Multi-contenedor

Suma de dos vectores distribuida en **múltiples contenedores Docker**, cada uno actuando como un nodo MPI independiente.

## Arquitectura

```
┌────────────┐      SSH       ┌────────────┐
│   master   │ ◄────────────► │  worker-1  │
│  (rank 0)  │                └────────────┘
│            │      SSH       ┌────────────┐
│  mpirun    │ ◄────────────► │  worker-2  │
│            │                └────────────┘
│            │      SSH       ┌────────────┐
│            │ ◄────────────► │  worker-3  │
└────────────┘                └────────────┘
```

- **master**: ejecuta `mpirun`, distribuye los datos y recolecta resultados.
- **worker-1, worker-2, worker-3**: ejecutan `sshd`, reciben su partición del vector y procesan la suma local.

Cada contenedor ejecuta **un proceso MPI** (un rank). La comunicación entre ellos ocurre por TCP a través de una red Docker compartida.

## Requisitos

- Docker
- Docker Compose

## Cómo ejecutar

### Construir y ejecutar (valores por defecto: N=200000, 4 nodos)

```bash
docker compose up --build
```

### Cambiar el tamaño del vector

```bash
N=1000000 docker compose up --build
```

### Detener y limpiar contenedores

```bash
docker compose down
```

### Reconstruir desde cero

```bash
docker compose down
docker compose up --build
```
