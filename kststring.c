/*
 * Arquivo: kststring.c
 * Desenvolvedor: Jayron Castro <jacastro@kstros.com>
 * Empresa: KSTROS.COM
 * Site: http://www.kstros.com
 * Data: 06/09/2015
 * Versão: 1.0
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
* Retornar a quantidade de vezes que o carectere informado foi encontrado
*/
int strContaCaractere(const char *p, const char cPesquisa){
	int iContador = 0;
	while(*p !=  '\0'){
		if (*p == cPesquisa){
			++iContador;
		}
		++p;
	}
	return iContador;
}

/*
* Retorna o indice da primeira ocorrência do caractere informado
*/
int strRetornaPrimeiraOcor(const char *p, const char cPesquisa){
	int iRet;
    int i;
    unsigned char booEncontrou = 0;
    
    int len = strlen(p);
    
    for(i=0;i<len;i++){
        if(p[i] == cPesquisa){
            booEncontrou = 1;
            break;
        }
    }
    
    iRet = (booEncontrou?i:-1);

	return iRet;
}

/*
* Retorna o indice da última ocorrência do caractere informado
*/
int strRetornaUltimaOcor(const char *p, const char cPesquisa){
	int iRet;
	char *ptr = strrchr(p,cPesquisa);
		if(*ptr){
			iRet = ptr - p;
		}else{
			iRet = -1;
		}
	return iRet;
}

/*
* Retorna uma substring com o tamanho informado partindo da esquerda
*/
char *strRetornaEsquerda(char *pDestino, const char *pOrigem, const int iComprimento){
	if(iComprimento<=strlen(pOrigem)){
		for(int i=0;i<iComprimento;i++){
			*pDestino++ = *pOrigem++;
		}
		*pDestino = '\0';
	}
	return pDestino;
}

/*
* Retorna uma substring apartir de um indice inicial e outro final
*/
char *strRetornaSubString(char *pDestino, const char *pOrigem, const int iIndiceIni, const int iIndiceFim){
	if ((iIndiceIni>=0)&&(iIndiceFim<=strlen(pOrigem))){
		pOrigem += iIndiceIni;
		int j = 0;
		for(int i=iIndiceIni;i<=iIndiceFim;i++){
			pDestino[j++] = *pOrigem++;
		}
		pDestino[j] = '\0';
	}
	return pDestino;
}

/*
* Retorna uma substring com o tamanho informado partindo da direita
*/
char *strRetornaDireita(char *pDestino, const char *pOrigem, const int iComprimento){
	if(iComprimento<=strlen(pOrigem)){
		int j = strlen(pOrigem);
		pOrigem += j - iComprimento;
		for(int i=0;i<iComprimento;i++){
			*pDestino++ = *pOrigem++;
		}
		*pDestino = '\0';
	}
	return pDestino;
}

/*
* Retorna uma string sem os espaços em branco no inicio e final
*/
char *strRetiraEspacoBranco(char *pString){
	char cTemp[strlen(pString) + 1];
	int iInicio, iFinal;
	
	//copia string de entrada para temporaria
	strcpy(cTemp,pString);
	
	//limpa string de entrada
	strcpy(pString,"");
	
	for(int i=0;i<strlen(cTemp);i++){
		if((cTemp[i] != ' ') && (cTemp[i] != '\t')){
			iInicio = i;
			break;
		}
	}
	
	for(int i=strlen(cTemp);i>0;i--){
		if((cTemp[i] != ' ') && (cTemp[i] != '\t')){
			iFinal = i;
			break;
		}
	}
	
	strRetornaSubString(pString,&cTemp[0],iInicio,iFinal);
	return pString;
}

/*
 * Retorna o indice da primeira ocorrencia numeral da esquerda para a direita
 * *pLinha -> recebe o conteudo do arquivo de interface
 */
int strRetornaPrimeiraOcorrenciaNumeral(const char *pLinha){
	int iRet = 0;
	while(*pLinha!='\0'){
		if(isdigit(*pLinha)){
			break;
		}
		iRet++;
		pLinha++;
	}
	return iRet;
}

/*
 * Compara duas variaveis do tipo char e retorna 0 se forem iguais e 1 caso sejam desiguais
 */
int strComparaChar(const char pChar1, const char pChar2){
	int iRet = 1;
	if(pChar1 == pChar2){
		iRet = 0;
	}
	return iRet;
}