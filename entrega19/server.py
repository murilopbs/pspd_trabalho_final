import socket
import subprocess

HOST = '127.0.0.1'
PORT = 65432

EXECUTABLE_PATH = './jogodavida_mpi_omp'
NUM_PROCESSES = '4'

print("--- Servidor de Processamento Iniciado ---")
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"Servidor escutando em {HOST}:{PORT}")
    print(f"Pronto para receber requisições e executar o backend: {EXECUTABLE_PATH}")

    while True:
        conn, addr = s.accept()
        with conn:
            print(f"\nConectado por {addr}")
            data = conn.recv(1024)
            if not data:
                print(f"Cliente {addr} desconectou.")
                continue

            message = data.decode('utf-8')
            print(f"Mensagem recebida: {message}")
            print(f"Executando o motor de backend...")
            
            try:
                command = ['mpiexec', '--oversubscribe', '--allow-run-as-root', '-n', NUM_PROCESSES, EXECUTABLE_PATH]
                result = subprocess.run(command, capture_output=True, text=True, timeout=120)
                
                if result.returncode == 0:
                    print("Execução do backend concluída com sucesso.")
                    response = result.stdout
                else:
                    print(f"Ocorreu um erro na execução do backend.")
                    response = f"ERRO:\n{result.stderr}"

            except Exception as e:
                print(f"Uma exceção ocorreu: {e}")
                response = f"ERRO no servidor: {e}"

            conn.sendall(response.encode('utf-8'))
            print("Resposta enviada ao cliente.")
