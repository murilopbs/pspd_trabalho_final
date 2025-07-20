import socket
import subprocess

HOST = '127.0.0.1'
PORT = 65432

print("--- Servidor de TESTE 'ls' iniciado ---")
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"Servidor de TESTE escutando em {HOST}:{PORT}")
    conn, addr = s.accept()
    with conn:
        print(f"Conectado por {addr}. Executando 'ls -l'...")
        try:
            # Executa um comando completamente inofensivo
            result = subprocess.run(['ls', '-l'], capture_output=True, text=True)
            print("Comando 'ls -l' executado com sucesso.")
            response = f"Resultado do 'ls -l':\n{result.stdout}"
        except Exception as e:
            print(f"O subprocess falhou: {e}")
            response = f"ERRO no servidor: {e}"
        
        conn.sendall(response.encode('utf-8'))
        print("Resposta enviada.")
