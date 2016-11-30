#Projeto Final de TR1

Para clonar o projeto, basta executar o comando no diretório desejado:

	git clone https://github.com/Yuri-Castro/TR1-Project---Partiu-SS.git

##Buildando o programa
Primeiro, copie o arquivo `main.cc` para pasta _scratch_ do ns-3:

	cd <diretorio_do_projeto>/TR1-Project---Partiu-SS/src
	cp main.cc <source_ns_3>/ns-3.25/scratch

Depois vá para pasta _ns-3.25_:

	cd <source_ns_3>/ns-3.25/scratch

E builde o programa com o ns-3:

	./waf clean
	./waf configure --build-profile=debug --enable-examples

##Executando o programa
Ainda na pasta _ns-3.25_:

	make run
