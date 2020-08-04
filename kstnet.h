/*
 * Arquivo: kstnet.h
 * Desenvolvedor: Jayron Castro <jacastro@kstros.com>
 * Empresa: KSTROS.COM
 * Site: http://www.kstros.com
 * Data: 06/09/2015
 * Versão: 1.0
 */

#ifndef KSTNET_H_
#define KSTNET_H_

#include "kstfailover.h"

int linkresponde(const Failover *, const unsigned char,  const unsigned char);
int validaEnderecoIp(const char *);
int validaOcteto(const char *);
int tabelaPossuiRota(const Link *);
void validaDuplicidadeLinks(Failover *);
void validaInterfaceLinks(Failover *);
char *retornaInterfaceLocal(char *, const char *);
char *retornaNomeLink(char *, const char *);
char *retornaInterfaceLink(char *, const char *);
char *retornaGatewayLink(char *, const char *);
char *retornaIpInterface(char *, const char *);
char *retornaMascaraCidr(char *, const char *);
char *retornaIdRede(char *, const char *, const char *);
char *converteCidrDecimal(char *, const unsigned char);
char *converteIPMaskDecBin(char *, const char *);
char *converteIPMaskBinDec(char *, const char *);
char *retornaIp(char *, const char *);

#endif
