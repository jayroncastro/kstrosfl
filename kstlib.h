/*
 * Arquivo: kstlib.h
 * Desenvolvedor: Jayron Castro <jacastro@kstros.com>
 * Empresa: KSTROS.COM
 * Site: http://www.kstros.com
 * Data: 06/09/2015
 * Versão: 1.0
 */

#ifndef KSTLIB_H_
#define KSTLIB_H_

//#include "kststruct.h"
//#include "kstconfig.h"
//#include "kstredundancia.h"

#include "kstfailover.h"

void criaArquivoInterface(char [6]);
int abreArquivoLeitura(const char *);
int abreArquivoEscrita(const char *);
void gravaLinhaNoArquivo(const char *);
void fechaArquivo();

int carregaLinks(Failover *);
int carregaConfigGeral(Failover *);
void organizaLinks(Failover *);
void carregaInfoLinks(Failover *);
void contaLinksCarregados(Failover *);
void backupRestoreIproute2(const Failover *);
int configuraIproute2(Failover *);
void carregaTabelaRota(const Link *);
int existeRegra(const Link *);

int arquivoExiste(const char *);

int existeGateway();
int existeInterface(const Link *);
void mudaGateway(const Link *);
void gatewayPincipal(Failover *);
void gatewayBackup(Failover *);

#endif
