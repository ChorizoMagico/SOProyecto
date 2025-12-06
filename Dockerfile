# Utiliza una imagen base de Ubuntu
FROM ubuntu:20.04

# Instalar dependencias necesarias
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    qemu \
    git \
    make \
    gcc \
    gdb \
    nasm \
    qemu-system-x86 \
    g++ \
    vim \
    curl

# Copiar el código fuente de xv6 al contenedor
COPY xv6-public /xv6

# Configurar el directorio de trabajo
WORKDIR /xv6

# Comando para compilar xv6
RUN make

# Comando por defecto para ejecutar xv6 con QEMU
CMD ["make", "qemu"]
