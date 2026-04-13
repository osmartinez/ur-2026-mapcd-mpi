FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        make \
        openmpi-bin \
        libopenmpi-dev \
        openssh-server \
        openssh-client \
    && rm -rf /var/lib/apt/lists/*

# Configurar SSH sin contraseña para comunicación MPI entre contenedores
RUN mkdir -p /run/sshd \
    && ssh-keygen -t rsa -b 2048 -f /root/.ssh/id_rsa -N "" \
    && cp /root/.ssh/id_rsa.pub /root/.ssh/authorized_keys \
    && chmod 600 /root/.ssh/authorized_keys \
    && echo "StrictHostKeyChecking no" >> /etc/ssh/ssh_config \
    && echo "UserKnownHostsFile /dev/null" >> /etc/ssh/ssh_config \
    && echo "LogLevel ERROR" >> /etc/ssh/ssh_config \
    && echo "PermitRootLogin yes" >> /etc/ssh/sshd_config

WORKDIR /app

COPY src/ src/
COPY Makefile .

RUN make build

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
