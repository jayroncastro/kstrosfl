/*
 * Arquivo: main.c
 * Desenvolvedor: Jayron Castro <jacastro@kstros.com>
 * Empresa: KSTROS.COM
 * Site: http://www.kstros.com
 * Data: 06/09/2015
 * Versão: 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include "kstlib.h"
#include "kstnet.h"
#include "kstfailover.h"
//Includes necessarios para daemon
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

Failover oFailOver;

int daemon_init(void){
    pid_t iPid;
    long iMaxFd;
    
    if((iPid = fork()) < 0){
        return -1;
    }
    
    if(iPid != 0){
        exit(0);
    }
    
    setsid();
    
    chdir("/");
    
    iMaxFd = sysconf(_SC_OPEN_MAX);
    
    for(int i=0;i<iMaxFd;i++){
        close(i);
    }
    
    return 0;
}

int main(int argc, char **argv) {
	char cArquivoConfigLinks[32] = "/etc/kstrosfl/opcoes.links.conf";	//arq. de configuracao dos links
	char cArquivoConfigGeral[26] = "/etc/kstrosfl/opcoes.conf";			//arq. de configuracao geral
	char cSysLog[200];													//variavel que armazena mensagem de log
	unsigned char booFailover;											//controla ativacao do failover
    unsigned char erro = 0;                                             //variavel que controle erro de ping
    
	//torna o processo em daemon
    if(daemon_init() < 0){
        perror(argv[0]);
        exit(errno);
    }
	//###########################
	
	strcpy(oFailOver.opcoes.arquivo.config,&cArquivoConfigGeral[0]);
	strcpy(oFailOver.opcoes.arquivo.links,&cArquivoConfigLinks[0]);
	strcpy(cSysLog,"");
	booFailover = 0;	//inicia desligado
	
	strcpy(cSysLog,"Carregando Daemon Kstrosfl...");
	syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);
    sprintf(cSysLog,"Lendo arquivo de configuracao geral em: %s",cArquivoConfigGeral);
	syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);
	if (!carregaConfigGeral(&oFailOver)){ //abre o arquivo de configuracao geral
        sprintf(cSysLog,"Lendo arquivo de configuracao de links em: %s",cArquivoConfigLinks);
        syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);
		if(!carregaLinks(&oFailOver)){ //carrega as informacoes do arq de conf. na matriz
			if(oFailOver.count > 1){
				booFailover = 1;	//liga o failover
				sprintf(cSysLog,"Processo de Failover ligado para [%u] links carregados!",oFailOver.count);
				syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);
			}else{
				strcpy(cSysLog,"Processo de Failover desligado, eh necessario ter mais de [1] link carregado!");
				syslog(LOG_DAEMON | LOG_ERR, cSysLog);
			}
		}else{
			strcpy(cSysLog,"Ocorreu erro na leitura ou interpretacao do arquivo /etc/kstrosfl/opcoes.links.conf");
			syslog(LOG_DAEMON | LOG_ERR, cSysLog);
		}
	}else{
		strcpy(cSysLog,"Nao foi encontrato o arquivo /etc/kstrosfl/opcoes.conf");
		syslog(LOG_DAEMON | LOG_ERR, cSysLog);
	}
	if(booFailover){
		backupRestoreIproute2(&oFailOver);
        if(!configuraIproute2(&oFailOver)){
            //###################################
            while(1){ 											// inicia loop do failover
                sleep((int)oFailOver.opcoes.atraso);			//cria tempo de atraso
                sprintf(cSysLog,"Iniciando teste de failover!!");
				syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);
                if(!(linkresponde(&oFailOver,1,0))){            //link atual responde normalmente
                    if(oFailOver.ativo != 0){                   //entra se o link ativo nao for o principal
                        if(!(linkresponde(&oFailOver,0,0))){    //entra se link principal responde
                            gatewayPincipal(&oFailOver);        //muda gateway para link principal
                        }else{                                  //entra se o link principal nao responde
                            sprintf(cSysLog,"Link de backup [%s] continua online e principal [%s] continua offline",
                            oFailOver.link[oFailOver.ativo].nome,oFailOver.link[0].nome);
                            syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);   //grava log da operacao
                        }
                    }else{
                        sprintf(cSysLog,"Link principal [%s] respondendo normalmente!",oFailOver.link[0].nome);
                        syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);
                    }
                }else{			                                       //link atual nao responde
					sprintf(cSysLog,"Link ativo [%s] nao respondendo!",oFailOver.link[oFailOver.ativo].nome);
					syslog(LOG_DAEMON | LOG_NOTICE, cSysLog);	//grava log
                    if(oFailOver.opcoes.erros == erro++){       //entra somente apos testar quant. de erros
						erro = 0;								//zera contador de erros
						if(oFailOver.ativo != 0){               //entra se o link ativo for o de backup
                            if(!(linkresponde(&oFailOver,0,0))){//entra se link principal responde
                                gatewayPincipal(&oFailOver);    //muda gateway para link principal
                            }else{                              //entra se link principal nao responde
								gatewayBackup(&oFailOver);		//muda gateway para prox. link backup disponivel
                            }
                        }else{                                  //entra se o link ativo for o principal
							gatewayBackup(&oFailOver);			//muda gateway para prox. link backup disponivel
                        }
                    }
                }
            }
            //###################################
        }else{
            sprintf(cSysLog,"Necessario permissao de escrita em %s",oFailOver.opcoes.arquivo.iproute);
            syslog(LOG_DAEMON | LOG_ERR, cSysLog);
        }
	}




	//exibe estrutura de arquivos

	/*printf("== Config.opcoes ==\n");
	printf("atraso: %u\n",oFailOver.opcoes.atraso);
	printf("ipalvo: %s\n",oFailOver.opcoes.ipalvo);
	printf("erros: %u\n",oFailOver.opcoes.erros);
	printf("== Config.Caminho ==\n");
	printf("Arquivo.Config: %s\n",oFailOver.opcoes.arquivo.config);
	printf("Arquivo.Links: %s\n",oFailOver.opcoes.arquivo.links);
	printf("Arquivo.Iproute: %s\n",oFailOver.opcoes.arquivo.iproute);
	printf("== Failover ==\n");
	printf("Failover.ativo: %u\n",oFailOver.ativo);
	printf("Failover.count: %u\n",oFailOver.count);
	printf("== Failover.Links ==\n");
	for(int j=0;j<5;j++){
		printf("link[%i].nome: %s\n",j,oFailOver.link[j].nome);
		printf("link[%i].eth: %s\n",j,oFailOver.link[j].eth);
		printf("link[%i].ip: %s\n",j,oFailOver.link[j].ip);
		printf("link[%i].maskCidr: %s\n",j,oFailOver.link[j].maskCidr);
		printf("link[%i].maskDecimal: %s\n",j,oFailOver.link[j].maskDecimal);
		printf("link[%i].gate: %s\n",j,oFailOver.link[j].gate);
		printf("link[%i].idRede: %s\n",j,oFailOver.link[j].idRede);
		printf("----\n");
	}*/
	
	return 0;
}
