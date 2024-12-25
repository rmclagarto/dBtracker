import sqlite3


# Função para criar o banco de dados, caso não exista
def criarbasedados():
	# Estabelece uma conexão com o banco de dados "dados.db"
	conexao = sqlite3.connect("dados.db")

	# Cria um objeto de cursor para executar comandos SQL
	cursor = conexao.cursor()
	
	# Executa uma instrução SQL para criar a tabela "sensordata" se não existir
	cursor.execute("""CREATE TABLE IF NOT EXISTS sensordata(
						id INTEGER PRIMARY KEY AUTOINCREMENT,
						db REAL,
						horas TEXT)""")
	
	# Confirma as alterações no banco de dados
	conexao.commit()
	
	# Fecha a conexão com o banco de dados
	conexao.close()
	
# Função para inserir dados no banco de dados
def inserir_dados(db,horas):
	# Estabelece uma conexão com o banco de dados "dados.db"
	conexao = sqlite3.connect("dados.db")

	# Cria um objeto de cursor para executar comandos SQL
	cursor = conexao.cursor()

	# Executa uma instrução SQL para inserir dados na tabela "sensordata"
	cursor.execute("INSERT INTO sensordata (db,horas) VALUES (?, ?)",(float(db),str(horas)))
	
	# Confirma as alterações no banco de dados
	conexao.commit()

	# Fecha a conexão com o banco de dados
	conexao.close()

# Função para visualizar todos os dados do banco de dados
def visualizar():
	# Estabelece uma conexão com o banco de dados "dados.db"
	conexao = sqlite3.connect("dados.db")

	# Cria um objeto de cursor para executar comandos SQL
	cursor = conexao.cursor()
	
	# Executa uma instrução SQL para selecionar todos os dados da tabela "sensordata" e ordená-los pelo campo "id" em ordem decrescente.
	cursor.execute('SELECT * FROM sensordata ORDER BY id DESC')

	# Recupera todos os dados da consulta
	dados = cursor.fetchall()
	
	# Fecha a conexão com o banco de dados
	conexao.close()

	# Retorna os dados recuperados
	return dados
