# Usar uma imagem base robusta e completa para garantir compatibilidade
FROM ubuntu:22.04

# Evitar prompts interativos durante a instalação
ENV DEBIAN_FRONTEND=noninteractive

# Instalar TODAS as dependências de uma só vez: Python, MPI, e Compilador C
RUN apt-get update && apt-get install -y \
    python3 \
    build-essential \
    openmpi-bin \
    libopenmpi-dev \
    && rm -rf /var/lib/apt/lists/*

# Definir o diretório de trabalho
WORKDIR /app

# Copiar todos os arquivos do projeto para o diretório de trabalho
COPY . .

# Compilar o código C dentro do contêiner
RUN mpicc -o jogodavida_mpi_omp jogodavida_mpi_omp.c -O3 -fopenmp

# Expor a porta do nosso servidor
EXPOSE 65432

# Definir o comando para iniciar o servidor Python quando o contêiner rodar
CMD ["python3", "./server.py"]
