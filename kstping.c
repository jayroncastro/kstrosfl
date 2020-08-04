/*
* Desenvolvedor: Jayron Castro
* Empresa: KSTROS.COM
* Site: http://www.kstros.com
* Data: 11/04/2016
* Versão: 1.0
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
* Testa se o link esta ativo, retornando:
* 0 -> link offline
* 1 -> link online
*/
int linkativo(const char *cIp){
	int iRet = 0, p[2], backup;
	char buf[1000], cComando[34];

	backup = dup(1);
	close(0);
	close(1);

	pipe(p);
	sprintf(cComando,"ping -c 2 -i 0.2 %s",cIp);
	system(cComando);
	dup2(backup, 1);

	int cont = 0;
	while (fgets(buf, 1000, stdin)){
		cont++;
	}

	if(cont==7){
		iRet = 1; //link ativo
	}

	return iRet;
}


