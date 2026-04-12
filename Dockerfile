FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        make \
        openmpi-bin \
        libopenmpi-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN make build

CMD ["mpirun", "--allow-run-as-root", "-np", "4", "/app/bin/vector_sum_mpi", "17"]
