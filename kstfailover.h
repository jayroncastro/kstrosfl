/*
 * Arquivo: kstfailover.h
 * Desenvolvedor: Jayron Castro <jacastro@kstros.com>
 * Empresa: KSTROS.COM
 * Site: http://www.kstros.com
 * Data: 13/10/2015
 * Versão: 1.0
 */

#ifndef KSTFAILOVER_H_
#define KSTFAILOVER_H_

#define IP 16

typedef struct kstcaminho{
	char config[26];	//arq. de configuracao geral
	char links[32]; 	//arq. de configuracao dos links
	char iproute[40];	//armazena o caminho para encontrar o arquivo rt_tables
} Caminho;

typedef struct kstconfig{
	unsigned char atraso; 		//tempo de atraso no teste do link ativo expresso em segundos
	char          ipalvo[IP]; 	//ip para ficar fazendo teste de conexao
	unsigned char erros; 		//quantidade de testes realizados antes da mudanca de link
	Caminho       arquivo;		//estrutura do tipo caminho
} Config;

typedef struct kstlink{
	char	nome[6]; 			//nome do link {5 caracteres para nome do link}
	char	eth[6]; 			//interface fisica onde o link está conectado {5 caracteres para interface}
	char	ip[IP]; 			//000.000.000.000
	char	maskCidr[3];		//00
	char	maskDecimal[IP];	//000.000.000.000
	char	gate[IP]; 			//000.000.000.000
	char	idRede[IP];			//000.000.000.000
} Link;

typedef struct kstfailover{
	unsigned char ativo;		//armazena o indice do link ativo -> [varia de 0 ate 4]
	unsigned char count;		//armazena a quantidade de links redundantes que foram carregados -> [varia de 1 ate 5]
	Config        opcoes;		//estrutura do tipo config
	Link		  link[5];		//estrutura do tipo link
} Failover;

#endif