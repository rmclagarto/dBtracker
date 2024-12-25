# Importação de módulos necessários
import http.server 
import socketserver
import os
import socket
import database
import threading

# Classe Var para FACILITAR A MANIPULAÇAO variáveis globais
class Var:
	def __init__(self):
		self.db = 0
		self.horas = ""
		
	def set_db(self,db):
		self.db = db
		
	def get_db(self):
		return self.db
		
	def set_horas(self,horas):
		self.horas = horas
	
	def get_horas(self):
		return self.horas

# Instância da classe Var
var = Var()

# Função para gerar o conteúdo HTML
def gerar_html(dados):
    # Cabeçalho do HTML com os estilos CSS fornecidos
    html_conteudo = """
    <!DOCTYPE html>
    <html lang="pt">
    <head>
      <meta charset="UTF-8">
      <title>DBtracking</title>
      <style>
        body {
          font-family: 'Arial', sans-serif;
          background-color: #015958;
          margin: 0;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          flex-direction: column;
          color: #fff; /* Cor do texto */
        }
        .info-container,
        .historico {
          background-color: #fff;
          border-radius: 10px;
          box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
          padding: 40px;
          text-align: center;
          width: 60%;
          margin-bottom: 20px;
        }
        .info-container {
          display: flex;
          justify-content: space-between;
        }
        .info-item {
          flex: 1;
          margin: 0 20px;
        }
        h1 {
          margin-top: 0;
          font-size: 24px;
          color: #000;
        }
        p {
          margin-bottom: 20px;
          color: #555;
        }
        strong {
          color: #f44336;
        }
        /* Estilos para o histórico */
        .historico {
          color: #000; /* Cor do texto */
          overflow-y: auto; /* Adicionando barra de rolagem vertical */
          max-height: 200px; /* Definindo a altura máxima antes de exibir a barra de rolagem */
        }
        /* Alinhar texto do histórico (excluindo o título) à esquerda */
        .historico p {
          text-align: left;
        }
        /* Alinhar o título do histórico ao centro */
        .historico h2 {
          text-align: center;
        }
      </style>
    </head>
    """
    html_conteudo +=f"""
   <body>
      <h1>DB Tracking</h1>
      <div class='info-container'>
        <div class='info-item'>
          <h1>Intensidade Sonora</h1>
          <p><strong>{var.get_db()} dB</strong></p>
        </div>
        <div class='info-item'>
          <h1>Data/Hora</h1>
          <p><strong>{var.get_horas()}</strong></p>
        </div>
      </div>
      <div class='historico'>
        <h2>Histórico</h2>
    """

    # Adicionando os dados recebidos na seção de histórico
    for linha in dados:
        html_conteudo += f"<p>Registro {linha[0]}: {linha[1]} dB - {linha[2]}</p>"

    # Finalizando o HTML
    html_conteudo += """
      </div>
      <script>
      setInterval(function(){
      location.reload();
      }, 5000);
      </script>
    </body>
    </html>
    """

    return html_conteudo	

# Função para iniciar o servidor web
def iniciar_servidor():
	# Define a porta do servidor
	porta=8000
	
  # Define uma classe MyHandler que herda de SimpleHTTPRequestHandler
	class MyHandler(http.server.SimpleHTTPRequestHandler):
		# Sobrescreve o método do_GET para personalizar o comportamento
		def do_GET(self):
			# Verifica se a solicitação é para a raiz do servidor
			if self.path == '/':
				
        # Obtém dados da função database.visualizar()
				dados = database.visualizar()
				
        # Gera uma página HTML com base nos dados
				pagina_html=gerar_html(dados)
				
        # Envia uma resposta HTTP de sucesso
				self.send_response(200)
				self.send_header('Content-type', 'text/html')
				self.end_headers()
				
        # Escreve o conteúdo HTML na resposta
				self.wfile.write(pagina_html.encode())
			else:
				# Se a solicitação não for para a raiz, chama o do_GET padrão
				super().do_GET()

	try:
		# Cria uma instância da classe MyHandler
		handler = MyHandler
		
		#manipulador = http.server.SimpleHTTPRequestHandler
		
    # Cria um servidor TCP na porta especificada, usando o manipulador MyHandler
    with socketserver.TCPServer(("",porta),handler) as httpd:
			
      # Exibe a mensagem indicando que o servidor está sendo executado
      print(f"http://localhost:{porta}")
	  
      # Mantém o servidor em execução continuamente
			httpd.serve_forever()
    except Exception as e:
	  
    # Captura e imprime qualquer exceção que ocorra durante a execução do servidor
    print(f"Erro (iniciar servidor){e}")	


# Função para receber dados externos por meio de sockets
def receber_dados():
	# Endereço IP do host
	host = "192.168.104.180"
	
	#netzinha.get_meuip()
	
  # Porta para escutar as conexões
	porta = 8080
	
  # Criação do socket TCP
	sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
	
	# Vincula o socket ao endereço e à porta especificados
  sock.bind((host,porta))

  # Coloca o socket em modo de escuta
	sock.listen(1)
	
  # Aceita a conexão do cliente
	conn,addr = sock.accept()
	
  # Loop para receber dados continuamente
	while True:
		# Recebe os dados do cliente (máximo de 1024 bytes)
		data = conn.recv(1024)
		
    # Verifica se não há mais dados, encerrando o loop se necessário
		if not data:
			break
		
    # Decodifica os dados recebidos, presumindo uma string separada por vírgula
		inf = data.decode('utf-8').split(',')
		

    # Atualiza as variáveis ou funções conforme necessário (depende das implementações de var e database)
		var.set_db(float(inf[0]))
		var.set_horas(str(inf[1]))
		
    # Insere os dados no banco de dados
		database.inserir_dados(inf[0],inf[1])
		
  # Fecha a conexão após o término do loop
	conn.close()
				
# Função para iniciar as threads do servidor e da recepção de dados
def sv_thread():				
	# Cria uma thread para a função iniciar_servidor
	servidor_thread = threading.Thread(target = iniciar_servidor)
	servidor_thread.start()
	
  # Cria uma thread para a função receber_dados
	dados_thread = threading.Thread(target = receber_dados)
	dados_thread.start()
