/*
 * Arquivo: kstlib.c
 * Desenvolvedor: Jayron Castro <jacastro@kstros.com>
 * Empresa: KSTROS.COM
 * Site: http://www.kstros.com
 * Data: 06/09/2015
 * Versão: 1.0
 */

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include "kststring.h"
#include "kstnet.h"
#include "kstlib.h"
#include "kstfailover.h"

#define LINHAS 5
#define COLUNAS 30

FILE	*arquivo;
char	*nome;
struct stat info;


//Cria arquivo local com informações da interface
void criaArquivoInterface(char cInterface[6]) {
	char	cComando[63];
	//sprintf(cComando,"ifconfig %s | grep 'inet ' > ./if-%s.tmp",cInterface,cInterface);
	sprintf(cComando,"ip -4 address ls %s | grep 'inet' > etc/kstrosfl/if-%s.tmp",cInterface,cInterface);
	//printf("Comando: %s\n",cComando);
	system(cComando);
};

/*
 * Abre arquivo especificado para leitura
 * Retorna 1 em caso de erro
 */
int abreArquivoLeitura(const char *pArquivo){
	int iRet = 0;
	arquivo = fopen(pArquivo,"r");
	if (arquivo == NULL){
		iRet = 1;
	}
	return iRet;
};

/*
 * Abre arquivo especificado para escrita
 * Retorna 1 em caso de erro
 */
int abreArquivoEscrita(const char *pArquivo){
	int iRet = 0;
	arquivo = fopen(pArquivo,"a");
	if (arquivo == NULL){
		iRet = 1;
	}
	return iRet;
};

/*
* Grava a linha no arquivo texto aberto
*/
void gravaLinhaNoArquivo(const char *pLinha){
	fwrite(pLinha,strlen(pLinha),1,arquivo);
}

/*
 * Fecha o arquivo aberto
 */
void fechaArquivo(){
	fclose(arquivo);
}

/*
 * Carrega as informações da interface que o link esta ligado
 * Tipo Failover *pFailOver
 */
void carregaInfoLinks(Failover *pFailOver){
	FILE *in;				//variavel que controla pipe
	extern FILE *popen();	//variavel que controla pipe
	char buffer[90];		//variavel que controla o buffer do pipe

	//limpa variaveis
	strcpy(buffer,"");

	for(int i=0;i<5;i++){
		if(strcmp(pFailOver->link[i].nome,"")){
			sprintf(buffer,"ip -4 address ls %s | grep 'inet'",pFailOver->link[i].eth);
			//in = popen(buffer,"r");
			if((in = popen(buffer,"r"))){
				while (fgets(buffer, sizeof(buffer), in)){
					//insere caractere fim de string
					buffer[strlen(buffer)-1] = '\0';
					strRetiraEspacoBranco(&buffer[0]);
					//carrega o IP
					retornaIpInterface(pFailOver->link[i].ip,&buffer[0]);
					//carrega a mascara em notacao cidr
					retornaMascaraCidr(pFailOver->link[i].maskCidr,&buffer[0]);
					//carrega a mascara em decimal
					converteCidrDecimal(pFailOver->link[i].maskDecimal,(unsigned char)atoi(pFailOver->link[i].maskCidr));
					//carrega ID da rede
					retornaIdRede(pFailOver->link[i].idRede,pFailOver->link[i].ip,pFailOver->link[i].maskDecimal);
				}
                //fecha o pipe
                pclose(in);
			}
		}
	}
}

/*
 * Conta a quantidade de links carregados e atualiza contador da estrutura
 */
void contaLinksCarregados(Failover *pFailOver){
	pFailOver->count = 0;
	for(int i=0;i<5;i++){
		if((strlen(pFailOver->link[i].nome)>0) && (strlen(pFailOver->link[i].eth)>0) && (strlen(pFailOver->link[i].ip)>0)
		&& (strlen(pFailOver->link[i].maskCidr)>0) && (strlen(pFailOver->link[i].maskDecimal)>0)
		&& (strlen(pFailOver->link[i].gate)>0) && (strlen(pFailOver->link[i].idRede)>0)){
			pFailOver->count++;
		}
	}
}

/*
 * Organiza a ordem dos links
 * Tipo Failover *pFailOver
 */
void organizaLinks(Failover *pFailOver){

	for(int i=0;i<5;i++){
		if(!(strcmp(pFailOver->link[i].nome,""))){
			for(int j=i+1;j<5;j++){
				if(strcmp(pFailOver->link[j].nome,"")){
					//organiza os dados em nova posicao
					strcpy(pFailOver->link[i].nome,pFailOver->link[j].nome);
					strcpy(pFailOver->link[i].eth,pFailOver->link[j].eth);
					strcpy(pFailOver->link[i].gate,pFailOver->link[j].gate);
					//apaga os dados da posicao antiga
					strcpy(pFailOver->link[j].nome,"");
					strcpy(pFailOver->link[j].eth,"");
					strcpy(pFailOver->link[j].gate,"");
					//forca saida do laco
					break;
				}
			}
		}
	}

}

/*
 * Carrega as informações do arquivo de configuração de links
 * Tipo Failover *pFailOver
 */
int carregaLinks(Failover *pFailOver){
	int iRet = 1;			//variavel que controla o retorno da funcao
	char cValor[20];		//variavel que armazena o valor temporario do campo
	FILE *in;				//variavel que controla pipe
	extern FILE *popen();	//variavel que controla pipe
	char buffer[200];		//variavel que controla o buffer do pipe
	char cSysLog[200];		//variavel que armazena mensagem de log

	//limpa variaveis
	strcpy(buffer,"");
	strcpy(cSysLog,"");

	//retira comentarios do arquivo de configuracao
	sprintf(buffer,"cat %s | grep -v '^#' | grep -v '^$'",pFailOver->opcoes.arquivo.links); //cria arquivo temp
	//printf("Comando: %s\n",buffer);

	int i = 0;
	char cNome[6];
	if((in = popen(buffer,"r"))){
		while (fgets(buffer, sizeof(buffer), in)){
			if(i<=4){
				buffer[strlen(buffer)-1] = '\0';	//insere caractere fim de string
				strcpy(cNome,"");	//limpa variaveis
				//carrega nome do link
				strRetornaEsquerda(cValor,buffer,strRetornaPrimeiraOcor(buffer,':'));
				if(strlen(cValor)>5){
					strRetornaEsquerda(cNome,cValor,5); //carrega nome do link com 5 caracteres
					strcpy(pFailOver->link[i].nome,cNome);
				}else{
					strcpy(pFailOver->link[i].nome,cValor);
				}
				//carrega interface do link
				strRetornaSubString(cValor,buffer,(strRetornaPrimeiraOcor(buffer,':')+1),
				(strRetornaUltimaOcor(buffer,':')-1));
				strcpy(pFailOver->link[i].eth,cValor);
				//carrega gateway do link
				strRetornaDireita(cValor,buffer,(strlen(buffer) - (strRetornaUltimaOcor(buffer,':')+1)));
				strcpy(pFailOver->link[i].gate,cValor);
			}
			i++; //última instrucao
		}
        //fecha o pipe
        pclose(in);
	}
	
	//valida links procurando duplicidade de nomes e interfaces
	validaDuplicidadeLinks(pFailOver);
	
	//valida links procurando por interfaces nao existentes no sistema
	validaInterfaceLinks(pFailOver);

	//organiza os links no vetor
	organizaLinks(pFailOver);

	//carrega as informacoes locais de cada link
	carregaInfoLinks(pFailOver);
	
	//conta os links carregados
	contaLinksCarregados(pFailOver);
	
	//executa teste para saber se houve erro no processo
	if(pFailOver->count > 0){
		iRet = 0;
		//informa no arquivo de log os links carregados
		for(int i=0;i<5;i++){
			if(strlen(pFailOver->link[i].nome)>0){
				sprintf(cSysLog,"Link [%s] na interface local [%s] com gateway [%s] carregado com sucesso!",
				pFailOver->link[i].nome,pFailOver->link[i].eth,pFailOver->link[i].gate);
				syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);
			}
		}
	}else{
		strcpy(cSysLog,"Nao foram carregados links para failover, verifique configuracao de rede.");
		syslog(LOG_DAEMON | LOG_ERR, cSysLog);
	}

	return iRet;
};

/*
 * Carrega as configuracoes gerais do aplicativo
 * Funcao retorna 0 teve sucesso e 1 houve erro
 */
int carregaConfigGeral(Failover *pFailOver){
	int  iRet = 1;			//variavel que controla o retorno da funcao
	int  iIndex;			//variavel que armazena o indice do caractere BRANCO
	char cVariavel[50];		//variavel que armazena o nome da variavel de configuracao
	char cValor[50];		//variavel que armazena o valor da variavel de configuracao
	char cControle[4];		//variavel que controla a configuracao carregada
	FILE *in;				//variavel que controla pipe
	extern FILE *popen();	//variavel que controla pipe
	char buffer[100];		//variavel que controla o buffer do pipe

	strcpy(cControle,"");	//limpa variavel de controle
	strcpy(buffer,"");		//limpa veriavel do buffer

	//retira comentarios do arquivo de configuracao
	sprintf(buffer,"cat %s | grep -v '^#' | grep -v '^$'",pFailOver->opcoes.arquivo.config);

	if((in = popen(buffer,"r"))){
		while (fgets(buffer, sizeof(buffer), in)){
			strcpy(cVariavel,"");	//limpa variavel
			strcpy(cValor,"");		//limpa valor
			buffer[strlen(buffer)-1] = '\0';	//insere caractere fim de string
			iIndex = strRetornaPrimeiraOcor(buffer,' ');
			strRetornaEsquerda(cVariavel,buffer,iIndex);
			strRetornaSubString(cValor,buffer,iIndex+1,(strlen(buffer)-1));
			if(!strcmp(cVariavel,"tempo_atraso")){
				if(isdigit(*cValor)){
					pFailOver->opcoes.atraso = (unsigned char)atoi(cValor);
					strcat(cControle,"1");
				}
			}else if(!strcmp(cVariavel,"endereco_pesquisa")){
				char Ip[16];
                strcpy(Ip,"");
				retornaIp(&Ip[0],&cValor[0]);
				strcpy(pFailOver->opcoes.ipalvo,&Ip[0]);
				strcat(cControle,"2");
			}else if(!strcmp(cVariavel,"contador_erros")){
				if(isdigit(*cValor)){
					pFailOver->opcoes.erros = (unsigned char)atoi(cValor);
					strcat(cControle,"3");
				}
			}else if(!strcmp(cVariavel,"caminho_iproute")){
				strcpy(pFailOver->opcoes.arquivo.iproute,cValor);
			}
            
		}
        //fecha o pipe
        pclose(in);
	}
    
	//carrega valor padrao
	if(!strcmp(cControle,"2")){ 		//**** deve carregar o valor padrao para 1 e 3 ****
		pFailOver->opcoes.atraso = 15;	//valor padrao para tempo de atraso
		pFailOver->opcoes.erros = 2;	//valor padrao para contador de erro
	}else if(!strcmp(cControle,"12")){
		pFailOver->opcoes.erros = 2;	//valor padrao para contador de erro
	}else if(!strcmp(cControle,"23")){
		pFailOver->opcoes.atraso = 15;	//valor padrao para tempo de atraso
	}

	//Valida somente se todos os campos existirem
	if((pFailOver->opcoes.atraso > 0) & (strlen(pFailOver->opcoes.ipalvo)>0) & 
	(pFailOver->opcoes.erros > 0) & (strlen(pFailOver->opcoes.arquivo.iproute)>0)){
		iRet=0;
	}

	return iRet;
}

/*
* Esta funcão testa se o arquivo existe
* pArquivo -> recebe um ponteiro representando o caminho do arquivo
* retorna 0 caso exista e 1 caso contrario
*/
int arquivoExiste(const char *pArquivo){
	int iRet = 1;
	if(!abreArquivoLeitura(pArquivo)){
		fclose(arquivo);
		iRet = 0;
	}
	return iRet;
}

/*
* Realiza o backup e restore do arquivo /etc/iproute2/rt_tables
*/
void backupRestoreIproute2(const Failover *pFailOver){
	char cComando[55] = "/opt/kstrosfl/rt_tables.ori";
	
	if(arquivoExiste(&cComando[0])){ //[nao existe arquivo] -> faz backup do arquivo rt_tables
		sprintf(cComando,"cp %s /opt/kstrosfl/rt_tables.ori",pFailOver->opcoes.arquivo.iproute);
		system(cComando);
	}else{ //[existe arquivo] -> copia rt_tables para /etc/iproute2/
		sprintf(cComando,"rm %s",pFailOver->opcoes.arquivo.iproute);
		system(cComando);
		sprintf(cComando,"cp /opt/kstrosfl/rt_tables.ori %s",pFailOver->opcoes.arquivo.iproute);
		system(cComando);
	}
}

/*
* Configura o arquivo /etc/iproute2/rt_tables conforme os links carregados
* A funcao retorna 0 no caso de sucesso na configuracao e 1 caso nao consiga abrir o arquivo
*/
int configuraIproute2(Failover *pFailOver){
    int iRet = 1;
	int iPrio = 100;
	char cLinha[10];
	char cComando[70];
    
    strcpy(cLinha,"");
    strcpy(cComando,"");
	
	if(!abreArquivoEscrita(pFailOver->opcoes.arquivo.iproute)){ //abre o arquivo rt_tables
        strcpy(cComando,"# Valores reservados para uso do kstrosfl\n");
        gravaLinhaNoArquivo(&cComando[0]); 	//escreve linha de comentario no arquivo
		for(int i=0;i<pFailOver->count;i++){
			sprintf(cLinha,"%i\t%s\n",iPrio,pFailOver->link[i].nome); //adiciona a tabela ao arquivo rt_tables
			gravaLinhaNoArquivo(&cLinha[0]); 	//escreve a linha no arquivo
			iPrio += 100;
		}
		
		fechaArquivo(); //fecha o arquivo rt_tables
        
        //elimina o gateway atual
		if(!existeGateway()){ //entra na opcao somente se o sistema possuir gate
			sprintf(cComando,"ip route del default");
			system(cComando);
		}
        
        //adic. gateway pelo link principal
		sprintf(cComando,"ip route add default via %s",pFailOver->link[0].gate);
		system(cComando);
		pFailOver->ativo = 0;	//armazena link ativo
        
        for(int i=0;i<pFailOver->count;i++){
			carregaTabelaRota(&(pFailOver->link[i])); //carrega a rota na tabela
        }
        
        iRet = 0; //seta variavel informando que nao houve erro no processamento
	}
    
    return iRet;
}

/*
* Carrega as rotas especificas por tabela de roteamento avancado
*/
void carregaTabelaRota(const Link *pLink){
	char cComando[70];
	
	//elimina a tabela da memoria
	strcpy(cComando,""); //limpa variavel da memoria
	sprintf(cComando,"ip route flush table %s",pLink->nome);
	//printf("Comando: %s\n",cComando);
	system(cComando);
	
	//elimina a regra da memoria
	if(!existeRegra(pLink)){ //entra somente se a regra existir na memoria
		strcpy(cComando,""); //limpa variavel da memoria
		sprintf(cComando,"ip rule del table %s",pLink->nome);
		//printf("Comando: %s\n",cComando);
		system(cComando);
	}
	
	//adiciona rota na tabela
	strcpy(cComando,""); //limpa variavel da memoria
	sprintf(cComando,"ip route add %s/%s dev %s src %s table %s",pLink->idRede,
	pLink->maskCidr,pLink->eth,pLink->ip,pLink->nome);
	//printf("Comando: %s\n",cComando);
	system(cComando);
	
	//adiciona rota na tabela
	strcpy(cComando,""); //limpa variavel da memoria
	sprintf(cComando,"ip route add default via %s table %s",pLink->gate,pLink->nome);
	//printf("Comando: %s\n",cComando);
	system(cComando);
	
	//adiciona regra da tabela no sistema
	strcpy(cComando,""); //limpa variavel da memoria
	sprintf(cComando,"ip rule add from %s table %s",pLink->ip,pLink->nome);
	//printf("Comando: %s\n",cComando);
	system(cComando);
}

/*
 * Esta funcao serve para testar se o sistema possui uma regra criada para a tabela informada
 * O retorno sera 0 caso exista uma regra configurada e 1 no caso contrario
 */
int existeRegra(const Link *pLink){
	FILE *in;				//variavel que controla pipe
	extern FILE *popen();	//variavel que controla pipe
	char buffer[200];		//variavel que controla o buffer do pipe
	int i = 0;				//contador

	//limpa variaveis
	strcpy(buffer,"");

	//retira comentarios do arquivo de configuracao
	sprintf(buffer,"ip rule show | grep %s",pLink->nome); //cria comando
	//printf("Comando: %s\n",buffer);

	if((in = popen(buffer,"r"))){
		while (fgets(buffer, sizeof(buffer), in)){
			buffer[strlen(buffer)-1] = '\0';	//insere caractere fim de string
			if(strlen(buffer)>0){
				i++; //incrementa contador
			}
		}
        //fecha o pipe
        pclose(in);
	}
	
	return (i==0?1:0);
}

/*
 * Esta funcao serve para testar se o sistema possui um gateway configurado
 * O retorno sera 0 caso exista um gateway configurado e 1 no caso contrario
 */
int existeGateway(){
	FILE *in;				//variavel que controla pipe
	extern FILE *popen();	//variavel que controla pipe
	char buffer[200];		//variavel que controla o buffer do pipe
	int i = 0;				//contador

	//limpa variaveis
	strcpy(buffer,"");

	//retira comentarios do arquivo de configuracao
	sprintf(buffer,"ip route show | grep default"); //cria comando
	//printf("Comando: %s\n",buffer);

	if((in = popen(buffer,"r"))){
		while (fgets(buffer, sizeof(buffer), in)){
			buffer[strlen(buffer)-1] = '\0';	//insere caractere fim de string
			if(strlen(buffer)>0){
				i++; //incrementa contador
			}
		}
        //fecha o pipe
        pclose(in);
	}
	
	return (i==0?1:0);
}

/*
 * Esta funcao serve para testar se a interface informada esta ativa
 * O retorno sera 0 caso exista um gateway configurado e 1 no caso contrario
 */
int existeInterface(const Link *pLink){
	FILE *in;				//variavel que controla pipe
	extern FILE *popen();	//variavel que controla pipe
	char buffer[200];		//variavel que controla o buffer do pipe
	int i = 0;				//contador

	//limpa variaveis
	strcpy(buffer,"");

	//retira comentarios do arquivo de configuracao
	sprintf(buffer,"ip route show | grep %s",pLink->eth); //cria comando
	//printf("Comando: %s\n",buffer);

	if((in = popen(buffer,"r"))){
		while (fgets(buffer, sizeof(buffer), in)){
			buffer[strlen(buffer)-1] = '\0';	//insere caractere fim de string
			if(strlen(buffer)>0){
				i++; //incrementa contador
			}
		}
        //fecha o pipe
        pclose(in);
	}
	
	return (i==0?1:0);
	
}

/*
* Funcao que muda o gateway para o link atual
*/
void mudaGateway(const Link *pLink){
    char cComando[70];
    
    //limpa variaveis
    strcpy(cComando,"");
    
    //elimina o gateway atual
	if(!existeGateway()){
		sprintf(cComando,"ip route del default");
		system(cComando);
	}
    
    //adic. gateway pelo link principal
    sprintf(cComando,"ip route add default via %s",pLink->gate);
    system(cComando);
}

/*
 * Funcao que muda o gateway para o link principal
 * (Failover)   -> recebe um objeto do tipo Failover
 */
void gatewayPincipal(Failover *pFailOver){
    char cSysLog[200];				//variavel que armazena mensagem de log
    Link lnk = pFailOver->link[0]; 	//armazena tipo de variavel link
    
    strcpy(cSysLog,"");
    
    mudaGateway(&lnk);             //muda gateway para link principal
    
	sprintf(cSysLog,"Link ativo alterado de backup [%s] para principal [%s]",
		pFailOver->link[pFailOver->ativo].nome,lnk.nome);
	syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);   //grava log da operacao
	
	pFailOver->ativo = 0;           //armazena link ativo
    
}

/*
 * Funcao que muda o gateway para o proximo link de backup disponivel
 * (Failover)   -> recebe um objeto do tipo Failover
 */
void gatewayBackup(Failover *pFailOver){
    int temp = (int)pFailOver->ativo;
    char cSysLog[200];							 //variavel que armazena mensagem de log
    Link lnk;
    
    strcpy(cSysLog,"");
    //printf("dentro\n");
    
    for(int i=1;i<pFailOver->count;i++){               //testa todos os links da backup
        if(!(linkresponde(pFailOver,0,i))){				//testa o link de backup informado
            lnk = pFailOver->link[i];                   //armazena tipo de variavel link
            mudaGateway(&lnk);                         	//muda gateway para link principal
            
            sprintf(cSysLog,"Link offline [%s] alterado para link online [%s]",
            pFailOver->link[temp].nome,lnk.nome);
			syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);   //grava log da operacao
			
			pFailOver->ativo = (unsigned char)i;        //armazena link ativo
			
			break;										//forca saida do loop
        }else{
			sprintf(cSysLog,"Link backup [%s] nao respondendo!",pFailOver->link[i].nome);
			syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);   //grava log da operacao
		}
    }
}
