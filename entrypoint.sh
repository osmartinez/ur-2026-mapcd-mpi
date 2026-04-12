#!/bin/bash
set -e

ROLE="${ROLE:-worker}"
N="${N:-200000}"
HOSTFILE="/app/hostfile"

if [ "$ROLE" = "worker" ]; then
    echo "[worker] Iniciando servidor SSH..."
    exec /usr/sbin/sshd -D
else
    echo "[master] Esperando a que los workers estén listos..."

    # Construir hostfile a partir de la variable WORKERS (lista separada por comas)
    IFS=',' read -ra WORKER_LIST <<< "$WORKERS"
    TOTAL_SLOTS=1  # el master cuenta como 1 slot

    # Primero agregar el master
    echo "master slots=1" > "$HOSTFILE"

    # Esperar a que cada worker tenga SSH listo
    for w in "${WORKER_LIST[@]}"; do
        echo "  Esperando a $w ..."
        for i in $(seq 1 30); do
            if ssh -o ConnectTimeout=2 "$w" true 2>/dev/null; then
                echo "  $w listo."
                break
            fi
            sleep 1
        done
        echo "$w slots=1" >> "$HOSTFILE"
        TOTAL_SLOTS=$((TOTAL_SLOTS + 1))
    done

    echo "[master] Hostfile:"
    cat "$HOSTFILE"
    echo ""
    echo "[master] Lanzando MPI con $TOTAL_SLOTS procesos, N=$N"

    exec mpirun \
        --allow-run-as-root \
        --hostfile "$HOSTFILE" \
        --mca btl_tcp_if_include eth0 \
        --mca oob_tcp_if_include eth0 \
        -np "$TOTAL_SLOTS" \
        /app/bin/vector_sum_mpi "$N"
fi
