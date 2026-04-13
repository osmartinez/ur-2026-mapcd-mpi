# Parte 1 - Suma de vectores con MPI

Programa básico que demuestra las operaciones colectivas de MPI: `MPI_Scatter`, `MPI_Reduce` y `MPI_Gather`.

> **Nota:** El número de procesos debe ser divisor de 16 (1, 2, 4, 8 o 16).

---

## Instalar MPI

### Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential openmpi-bin libopenmpi-dev
```

### macOS

```bash
brew install open-mpi
```

### Verificar instalación

```bash
mpicc --version
mpirun --version
```

---

## Compilar

```bash
cd parte-1
mpicc -o suma_mpi main.c
```

---

## Ejecutar

```bash
# Con 4 procesos
mpirun -np 4 ./suma_mpi

# Con 2 procesos
mpirun -np 2 ./suma_mpi

# Con 8 procesos
mpirun -np 8 ./suma_mpi
```

### Salida esperada (con 4 procesos)

```
Suma global = 136
Datos reunidos: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
```

---

## Operaciones MPI utilizadas

| Operación | Descripción |
|-----------|-------------|
| `MPI_Scatter` | Reparte el vector desde rank 0 a todos los procesos |
| `MPI_Reduce` | Combina las sumas locales en una suma global (en rank 0) |
| `MPI_Gather` | Reúne los datos locales de vuelta en rank 0 |
