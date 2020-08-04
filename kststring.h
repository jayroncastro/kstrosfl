/*
 * Arquivo: kststring.c
 * Desenvolvedor: Jayron Castro <jacastro@kstros.com>
 * Empresa: KSTROS.COM
 * Site: http://www.kstros.com
 * Data: 06/09/2015
 * Versão: 1.0
 */

#ifndef KSTSTRING_H_
#define KSTSTRING_H_

int strContaCaractere(const char *, const char);
int strRetornaPrimeiraOcor(const char *, const char);
int strRetornaUltimaOcor(const char *, const char);
char *strRetornaEsquerda(char *, const char *, const int);
char *strRetornaSubString(char *, const char *, const int, const int);
char *strRetornaDireita(char *, const char *, const int);
char *strRetiraEspacoBranco(char *);
int strRetornaPrimeiraOcorrenciaNumeral(const char *);
int strComparaChar(const char, const char);

#endif
