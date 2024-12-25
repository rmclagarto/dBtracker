import os
from tkinter import *
import database
import servidor
import socket

comprimento = 200
altura =100
				
def captarIP(caixa_ip):
	ip = caixa_ip.get()
	if ip.count('.') == 3:
		servidor.sv_thread(ip)
		#app.destroy()

def main():
	"""
	def on_button_click():
		captarIP(caixa_ip)
	"""
		
	if not os.path.exists("dados.db"):
		database.criarbasedados()
		
	"""
	app = Tk()
	app.geometry(f"{comprimento}x{altura}")
	app.resizable(False,False)
	label = Label(app,text="Introduz o IP:")
	label.pack()
	caixa_ip = Entry(app,width=20)
	caixa_ip.pack()
	botao = Button(app,text="Confirmar",command=on_button_click)
	botao.pack()
	app.mainloop()
	"""
	servidor.sv_thread()
	

if __name__=="__main__":
	main()