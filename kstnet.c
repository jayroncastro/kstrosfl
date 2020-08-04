/*
 * Arquivo: kstnet.c
 * Desenvolvedor: Jayron Castro <jacastro@kstros.com>
 * Empresa: KSTROS.COM
 * Site: http://www.kstros.com
 * Data: 06/09/2015
 * Versão: 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "kststring.h"
#include "kstnet.h"
#include "kstfailover.h"
#include "kstlib.h"

#define h_addr h_addr_list[0]

/*
 * Esta funcao testa se o link responde
 * Retorno pode ser: "0" -> respondendo normalmente ou "1" -> nao respondendo
 * Argumentos:
 * (*pFailOver) -> recebe um ponteiro para o tipo de dados Failover
 * (linkAtivo)  -> caso o valor seja 1 o teste sera feito no link atual e caso o valor for 0 o teste sera realizado no link informado
 * (index)      -> recebe o indice do link a ser testatado, caso o teste nao seja no link ativo
 */
int linkresponde(const Failover *pFailOver, const unsigned char linkAtivo,  const unsigned char index){
    FILE *in;				//variavel que controla pipe
	extern FILE *popen();	//variavel que controla pipe
	char buffer[100];		//variavel que controla o buffer do pipe
	int iRet = 1;			//variavel que controla estado do link
	int iGateway;			//variavel que controla se existe gateway default
	int iControle;			//variavel que controla a criacao do comando ping
	Link lnk;				//objeto do tipo Link
	
	//limpa variaveis
	strcpy(buffer,"");
	iGateway = 1;			//possui gateway default
	
	lnk = pFailOver->link[(linkAtivo?pFailOver->ativo:index)];
	iControle = (int)((!existeInterface(&lnk))?1:0);
	
	//caso nao exista rotamento para a interface sera criado
	if(tabelaPossuiRota(&lnk)){
		carregaTabelaRota(&lnk); //carrega a rota na tabela
	}
	
	if(iControle){		//entra somente se existir a interface
		if(linkAtivo){  //faz o teste de resposta no link atual
			if(!existeGateway()){	//entra somente se existir gateway default
				sprintf(buffer,"ping -c 2 -i 0.2 %s | grep %c",pFailOver->opcoes.ipalvo,'%');
			}else{
				iGateway = 0;		//nao possui gateway default
				sprintf(buffer,"ping -c 2 -i 0.2 -I %s %s | grep %c",pFailOver->link[pFailOver->ativo].ip,pFailOver->opcoes.ipalvo,'%');
			}
		}else{          //faz o teste de resposta em link especifico
			sprintf(buffer,"ping -c 2 -i 0.2 -I %s %s | grep %c",pFailOver->link[index].ip,pFailOver->opcoes.ipalvo,'%');
		}
		//printf("Comando: %s\n",buffer);
		
		if((in = popen(buffer,"r"))){
			char buffer1[50];	//recebe parte do retorno do comando ping
			int indice = -1;	//inicializa a variavel indice
			strcpy(buffer1,"");	//limpa variavel
			while (fgets(buffer, sizeof(buffer), in)){
				buffer[strlen(buffer)-1] = '\0';	//insere caractere fim de string
				strRetiraEspacoBranco(&buffer[0]);	//retira espacos em branco da string
				//printf("Comando1: %s\n",buffer);
				indice = strRetornaPrimeiraOcor(&buffer[0],'%');	//retorna indice correspondente ao caractere %
				if(indice > -1){	//entra somente se existir o caractere percentual
					strRetornaEsquerda(&buffer1[0],&buffer[0],indice);	//quebra retorno do ping ate caractere %
					//printf("Parte1: %s\n",buffer1);
					//printf("Linha: %i -> %s\n",indice,buffer);
					strcpy(buffer,"");	//limpa variavel
					indice = strRetornaUltimaOcor(&buffer1[0],',');		//retorna indice da ultima ocorrencia ','
					//printf("Linha: %i -> %s\n",indice,buffer1);
					strRetornaDireita(&buffer[0],&buffer1[0],strlen(buffer1) - (indice + 1));	//Isola percentual
					strRetiraEspacoBranco(&buffer[0]);	//retira espacos em branco da string
					iRet = atoi(buffer);	//converte resultado em numero inteiro
					//printf("Percentual: %i\n",iRet);
				}
			}
			//fecha o pipe
			pclose(in);
		}
		
		if((iRet<100)&(iGateway)){
			iRet = 0; //link online
		}else{
			iRet = 1; //link offline
		}
	}else{
		iRet = 1; //link offline
	}
	
	//printf("Retorno => %i, Gateway => %i\n",iRet,iGateway);

	return iRet;
}
/*
* Testa se o link esta ativo, retornando:
* 1 -> link offline
* 0 -> link online
*/
/*int linkativo(const Link *pLink){
	FILE *in;				//variavel que controla pipe
	extern FILE *popen();	//variavel que controla pipe
	char buffer[100];		//variavel que controla o buffer do pipe
	int iRet = 1;			//variavel que controla estado do link
	int cont = 0;			//variavel que controla o retorno do ping
	
	
	//limpa variaveis
	strcpy(buffer,"");
	
	sprintf(buffer,"ping -c 2 -i 0.2 %s",pLink->ip);
	
	if((in = popen(buffer,"r"))){
		while (fgets(buffer, sizeof(buffer), in)){
			cont++;
		}
        //fecha o pipe
        pclose(in);
	}

	if(cont==7){
		iRet = 0; //link ativo
	}

	return iRet;
}*/

/*
* Valida um endereço IP
* 0 -> Retorna caso o endereço seja válido
* 1 -> Retorna caso o endereço seja inválido
*/
int validaEnderecoIp(const char *cEndIp){
	int j = 0, iRet = 1, l = strlen(cEndIp);
	char cOcteto[4];
	if(strContaCaractere(cEndIp,'.')==3){
		for(int i=0;i<l;i++){
			if(*cEndIp!='.'){
				cOcteto[j++] = *cEndIp;
			}else{
				cOcteto[j] = '\0';
				j = 0;
				iRet = validaOcteto(cOcteto);
			}
			if(i<(l-1)){
				cEndIp++;
			}else{
				cOcteto[j] = '\0';
				iRet = validaOcteto(cOcteto);
			}
		}
	}
	return iRet;
}

/*
* Valida o octeto informado
* 0 -> octeto válido
* 1 -> octeto inválido
*/
int validaOcteto(const char *p){
	int iRet = 1;
	int iControle = 0;
	int iComprimento = strlen(p);
	while(*p!='\0'){
		if(isdigit(*p++)){
			iControle++;
		}
	}
	if(iControle==iComprimento){
		p -= iControle;
		if((atoi(p)>=0) && (atoi(p)<=255)){
			iRet = 0;
		}
	}
	return iRet;
}

/*
 * Esta funcao serve para testar se o sistema possui uma rota para a tabela
 * O retorno sera 0 caso exista uma regra configurada e 1 no caso contrario
 */
int tabelaPossuiRota(const Link *pLink){
	FILE *in;				//variavel que controla pipe
	extern FILE *popen();	//variavel que controla pipe
	char buffer[200];		//variavel que controla o buffer do pipe
	int i = 0;				//contador

	//limpa variaveis
	strcpy(buffer,"");

	//retira comentarios do arquivo de configuracao
	sprintf(buffer,"ip route show table %s | grep default",pLink->nome); //cria comando
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
* Pesquisa todos os nomes dos links a procura de nomes repetidos e exclui as duplicatas
*/
void validaDuplicidadeLinks(Failover *pFailOver){
	int contador;

	//exclui nomes de links duplicados
	for(int i=0;i<5;i++){
		contador = 0;
		for(int j=0;j<5;j++){
			if(!(strcmp(pFailOver->link[i].nome,pFailOver->link[j].nome))){
				contador++;
				if(contador>1){
					strcpy(pFailOver->link[j].nome,"");	//limpa o nome duplicado
					strcpy(pFailOver->link[j].eth,"");	//limpa interface do nome duplicado
					strcpy(pFailOver->link[j].gate,"");	//limpa gateway do nome duplicado
					contador--;
				}
			}
		}
	}

	//exclui interfaces de rede duplicadas
	for(int i=0;i<5;i++){
		contador = 0;
		for(int j=0;j<5;j++){
			if(!(strcmp(pFailOver->link[i].eth,pFailOver->link[j].eth))){
				contador++;
				if(contador>1){
					strcpy(pFailOver->link[j].nome,"");	//limpa o nome duplicado
					strcpy(pFailOver->link[j].eth,"");	//limpa interface do nome duplicado
					strcpy(pFailOver->link[j].gate,"");	//limpa gateway do nome duplicado
					contador--;
				}
			}
		}
	}

}

/*
 * Valida as interfaces dos links informados, procurando por interfaces zumbis
 */
void validaInterfaceLinks(Failover *pFailOver){
	FILE *in;				//variavel que controla pipe
	extern FILE *popen();	//variavel que controla pipe
	char buffer[90];		//variavel que controla o buffer do pipe
	char mIface[5][6];		//matriz para armazenar interfaces locais
	char cIface[6];
	unsigned char i, m;
	
	//limpa variaveis
	strcpy(buffer,"");
	for(int j=0;j<5;j++){
		strcpy(mIface[j],"");
	}
	
	//inicializa variaveis
	i = 0;
	
	//cria comando
	strcpy(buffer,"netstat -i");
	
	//pesquisa todas as interfaces de rede locais
	if((in = popen(buffer,"r"))){
		while (fgets(buffer, sizeof(buffer), in)){
			buffer[strlen(buffer)-1] = '\0';	//insere caractere fim de string
			strRetiraEspacoBranco(&buffer[0]);
			retornaInterfaceLocal(cIface,&buffer[0]);
			strcpy(mIface[i],cIface);	//armazena as interfaces locais na matriz
			i++;
		}
        //fecha o pipe
        pclose(in);
	}
	
	//exclui interfaces inexistentes
	for(int j=0;j<5;j++){
		m = 0;
		if(strcmp(pFailOver->link[j].eth,"")){
			for(int l=0;l<i;l++){
				if(!(strcmp(pFailOver->link[j].eth,mIface[l]))){
					m++;
					break;
				}
			}
			if(!m){
				strcpy(pFailOver->link[j].nome,"");	//limpa o nome duplicado
				strcpy(pFailOver->link[j].eth,"");	//limpa interface do nome duplicado
				strcpy(pFailOver->link[j].gate,"");	//limpa gateway do nome duplicado
			}
		}
	}
}

/*
* Retorna o nome do link
*/
char *retornaNomeLink(char *pNome, const char *pLink){
	int j = strRetornaPrimeiraOcor(pLink,':');
	strRetornaEsquerda(pNome,pLink,j);
	return pNome;
}

/*
* Retorna a interface do link
*/
char *retornaInterfaceLink(char *pInterface, const char *pLink){
	int j = strRetornaPrimeiraOcor(pLink,':');
	int l = strRetornaUltimaOcor(pLink,':');
	strRetornaSubString(pInterface,pLink,++j,--l);
	return pInterface;
}

/*
 * retorna o nome da interface do S.O. conforme linha do comando "netstat -i"
 */
char *retornaInterfaceLocal(char *pIface, const char *pBuffer){
	char cBranco = ' ';	//caractere branco
	unsigned char cFinalLinha = 0;
	
	for(int i=0;i<5;i++){
		if(strComparaChar(pBuffer[i],cBranco)){
			*pIface = pBuffer[i];
		}else{
			cFinalLinha = 1;
			*pIface = '\0';
			break;
		}
		pIface++;
	}
	if(!cFinalLinha){
		*pIface = '\0';
	}
	
	return pIface;
}

/*
* Retorna o gateway do link
*/
char *retornaGatewayLink(char *pGateway, const char *pLink){
	int iComprimento = strlen(pLink);
	int iCuringa = strRetornaUltimaOcor(pLink,':');
	iComprimento = (iComprimento - iCuringa) - 1;
	strRetornaDireita(pGateway,pLink,iComprimento);
	return pGateway;
}

/*
 * Retorna o endereco IP da interface informada
 * *pEnderecoIp -> ponteiro que armazena o endereco de IP
 * pLinha -> Recebe o ponteiro de vetor para a linha da interface
 */
char *retornaIpInterface(char *pEnderecoIp, const char *pBuffer){
	int j = strRetornaPrimeiraOcorrenciaNumeral(pBuffer);
	for(int i=j;i<strlen(pBuffer);i++){
		if(pBuffer[j]!='/'){
			*pEnderecoIp++ = pBuffer[j];
			j++;
		}else{
			*pEnderecoIp = '\0';
			break;
		}
	}
	return pEnderecoIp;
}

/*
 * Retorna a mascara em notacao cidr da interface informada
 * *pEnderecoIp -> ponteiro que armazena o endereco de IP
 * pLinha -> Recebe o ponteiro de vetor para a linha da interface
 */
char *retornaMascaraCidr(char *pCidr, const char *pBuffer){
	int iIni, iFin;
	//char cCidr[3];
	iIni = strRetornaPrimeiraOcor(pBuffer,'/');
	iFin = iIni + 2;
	strRetornaSubString(pCidr,pBuffer,++iIni,iFin);
	//iRet = atoi(cCidr);
	return pCidr;
}

/*
 * Retorna o ID da rede sendo informado IP e Mascara em formato decimal
 * pIdRede	-> ponteiro que armazena o ID da rede {saida}
 * pIpDec	-> ponteiro que recebe o IP da rede {entrada}
 * pMaskDec	-> ponteiro que recebe a mascara da rede {entrada}
 */
char *retornaIdRede(char *pIdRede, const char *pIpDec, const char *pMaskDec){
	char cIpBin[36];
	char cMaskBin[36];
	char cIdBin[36];
	
	//limpar variaveis
	strcpy(cIpBin,"");
	strcpy(cMaskBin,"");
	strcpy(cIdBin,"");
	
	//converte IP para binario
	converteIPMaskDecBin(&cIpBin[0],pIpDec);
	//converte mascara para binario
	converteIPMaskDecBin(&cMaskBin[0],pMaskDec);
	//calcula e armazena a ID em binario
	for(int i=0;i<36;i++){
		if(cMaskBin[i] == '1'){
			cIdBin[i] = cIpBin[i];
		}else if(cMaskBin[i] == '0'){
			cIdBin[i] = cMaskBin[i];
		}else if(cMaskBin[i] == '\0'){
			cIdBin[i] = '\0';
		}else{
			if(i<33){
				cIdBin[i] = '.';
			}
		}
	}
	
	//converte ID binario em decimal
	converteIPMaskBinDec(pIdRede,&cIdBin[0]);
	
	return pIdRede;
}

/*
 * Retorna a mascara em formato decimal
 * *pMascara -> ponteiro de saida da mascara convertida
 * *pCidr -> ponteiro de entrada com a mascara em notacao cidr
 */
char *converteCidrDecimal(char *pMascara, const unsigned char iCidr){
	switch(iCidr){
		case 30:
			strcpy(pMascara,"255.255.255.252\0");
			break;
		case 29:
			strcpy(pMascara,"255.255.255.248\0");
			break;
		case 28:
			strcpy(pMascara,"255.255.255.240\0");
			break;
		case 27:
			strcpy(pMascara,"255.255.255.224\0");
			break;
		case 26:
			strcpy(pMascara,"255.255.255.192\0");
			break;
		case 25:
			strcpy(pMascara,"255.255.255.128\0");
			break;
		case 24:
			strcpy(pMascara,"255.255.255.0\0");
			break;
		case 23:
			strcpy(pMascara,"255.255.254.0\0");
			break;
		case 22:
			strcpy(pMascara,"255.255.252.0\0");
			break;
		case 21:
			strcpy(pMascara,"255.255.248.0\0");
			break;
		case 20:
			strcpy(pMascara,"255.255.240.0\0");
			break;
		case 19:
			strcpy(pMascara,"255.255.224.0\0");
			break;
		case 18:
			strcpy(pMascara,"255.255.192.0\0");
			break;
		case 17:
			strcpy(pMascara,"255.255.128.0\0");
			break;
		case 16:
			strcpy(pMascara,"255.255.0.0\0");
			break;
		default: //Caso seja passada uma mascara menor que 16 - [gera erro]
			strcpy(pMascara,"0.0.0.0\0");
			break;
	}
	return pMascara;
}

/*
 * Retorna um ponteiro com ou IP ou Mascara em formato binario
 * *pIpMaskDec -> Um ponteiro de um char[16] tipo 000.000.000.000 {entrada}
 * *pIpMaskBin -> Um ponteiro de um char[36] tipo 00000000.00000000.00000000.00000000 {saida}
 */
char *converteIPMaskDecBin(char *pIpMaskBin, const char *pIpMaskDec){
	char mIpDec[4][4];
	char mIpBin[4][9];
	unsigned char iLinha, iColuna;
	unsigned char octeto;
	unsigned int divisor;
	
	//limpar variaveis
	for(int i=0;i<4;i++){
		strcpy(mIpDec[i],"");
		strcpy(mIpBin[i],"");
	}
	iLinha = 0;
	iColuna = 0;
	octeto = 0;
	divisor = 0;
	
	//Divide os octetos decimais em uma matriz 4X4
	if(!validaEnderecoIp(pIpMaskDec)){
		for(int i=0;i<strlen(pIpMaskDec);i++){
			if(pIpMaskDec[i] != '.'){
				mIpDec[iLinha][iColuna] = pIpMaskDec[i];
				iColuna++;
				if(i==(strlen(pIpMaskDec)-1)){
					mIpDec[iLinha][iColuna] = '\0';
				}
			}else{
				mIpDec[iLinha][iColuna] = '\0';
				iLinha++;
				iColuna = 0;
			}
		}
	}
	
	//Converte para binario e armazena em matriz
	for(int i=0;i<4;i++){
		octeto = (unsigned char)atoi(mIpDec[i]);
		divisor = 256;
		for(int j=0;j<=8;j++){
			divisor /= 2;
			if(divisor>0){
				if(octeto>=divisor){
					mIpBin[i][j] = '1';
					octeto -= divisor;
				}else{
					mIpBin[i][j] = '0';
				}
			}else{
				mIpBin[i][j] = '\0';
			}
		}
	}
	
	//carrega IP binario na variavel de retorno
	for(int i=0;i<4;i++){
		for(int j=0;j<8;j++){
			*pIpMaskBin = mIpBin[i][j];
			pIpMaskBin++;
		}
		if(i<3){
			*pIpMaskBin = '.';
			pIpMaskBin++;
		}else{
			*pIpMaskBin = '\0';
		}
	}

	return pIpMaskBin;
}

/*
 * Retorna um ponteiro com ou IP ou Mascara em formato decimal
 * *pIpMaskBin -> Um ponteiro de um char[36] tipo 00000000.00000000.00000000.00000000 {entrada}
 * *pIpMaskDec -> Um ponteiro de um char[16] tipo 000.000.000.000 {saida}
 */
char *converteIPMaskBinDec(char *pIpMaskDec, const char *pIpMaskBin){
	unsigned char mIpDec[4];
	char mIpBin[4][9];
	char cIdDec[16];
	unsigned char iLinha = 0;
	unsigned char iColuna = 0;
	unsigned int divisor = 0;
	char bit[2];
	
	//limpar variaveis
	for(int i=0;i<4;i++){
		mIpDec[i] = 0;
		strcpy(mIpBin[i],"");
	}
	strcpy(cIdDec,"");
	
	//Divide os octetos decimais em uma matriz 4X9
	for(int i=0;i<strlen(pIpMaskBin);i++){
		if(pIpMaskBin[i] != '.'){
			mIpBin[iLinha][iColuna] = pIpMaskBin[i];
			iColuna++;
			if(i==(strlen(pIpMaskBin)-1)){
				mIpBin[iLinha][iColuna] = '\0';
			}
		}else{
			mIpBin[iLinha][iColuna] = '\0';
			iLinha++;
			iColuna = 0;
		}
	}
	
	//Converte para binario e armazena em matriz
	for(int i=0;i<4;i++){
		strcpy(bit,"");
		divisor = 256;
		for(int j=0;j<8;j++){
			divisor /= 2;
			bit[0] = mIpBin[i][j];
			bit[1] = '\0';
			mIpDec[i] += (unsigned char)(atoi(bit) * divisor);
		}
	}
	
	//carrega ID decimal na variavel de retorno
	sprintf(pIpMaskDec,"%u.%u.%u.%u",mIpDec[0],mIpDec[1],mIpDec[2],mIpDec[3]);

	return pIpMaskDec;
}

/*
 * Retorna um IP apartir de um nome valido na rede
 * *pIp   -> 
 * *pNome -> 
 */
char *retornaIp(char *pIp, const char *pNome){
	struct hostent *h;

	h = gethostbyname(pNome);
	
	if(h!=NULL){
		strcpy(pIp,inet_ntoa(*(struct in_addr *)h->h_addr));
	}

	if((strlen(pIp))==0){
		strcpy(pIp,"8.8.8.8");
	}
	
	return pIp;
}
