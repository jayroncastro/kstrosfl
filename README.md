# kstrosfl
Failover de links

1 - Apresentação:

    Esta ferramenta trata sobre o processo de failover em sistemas operacionais GNU/Linux, mais precisamente na distribuição Debian.
    
    Atualmente existem várias ferramentas embarcadas para solucionar este problema em uma rede, ferramentas estas que são conhecidas tecnicamente como "apliance" para o sistema operacional e dependem de um aplicativo externo, uma outra ferramente bem difundida na internet é um script chamado "gwping" com suas variações e pode ser baixado também no repositório GitHub.
    
    Particularmente achei sua implementação muito complicada e por esse motivo, tomei a decisão de desenvolver um aplicativo que tornar-se muito fácil a implementação do processo da failover em uma rede e de forma ao aplicativo trabalhar como um daemon nativo do sistema.
    
    A ferramenta foi desenvolvida sob o codinome "kstrosfl" e possui um monitoramento ativo para até 5 links de dados.

2 - A ferramenta
    
    A ferramenta "kstrosfl" é formada por 4 arquivos principais, listados abaixo em seus respectivos diretórios:
    
    /etc/kstrosfl/opcoes.conf -> este arquivo é responsável por armazenar as configurações gerais
    /etc/kstrosfl/opcoes.links.conf -> este arquivo serve para armazenar as configurações dos links redundantes
    /usr/sbin/kstrosfl -> este arquivo é o binário
    /etc/init.d/kstrosfl -> este arquivo serve para iniciar ou parar a ferramenta
    
    + O arquivo opcoes.conf -> Nesse arquivo ficam armazenadas as configurações gerais para a ferramenta, até o presente momento controla somente 4 variáveis para a ferramenta.
    
    Relação de variáveis do arquivo opcoes.conf:
    
    tempo_atraso -> esta variável deve receber um valor inteiro e maior que zero expressando os segundos, essa variável não é obrigatória, pois caso não seja informada kstrosfl carrega o valor padrão de 20 segundos para atraso nos testes dos links, não é recomendado reduzir esse valor para kstrosfl não ter o chamado falso positivo nos testes;
    
    endereco_pesquisa -> esta variável deve receber um valor do tipo string representando um host externo a rede local, por padrão o endereço www.google.com.br vem configurado, este campo é obrigatório e será o endereço que o kstrosfl ficará testando;
    
    contador_erros -> esta variável deve receber um valor inteiro e maior que zero expressando a quantidade de erros que kstrosfl irá aceitar antes de mudar o status do link, essa variável não é obrigatória, pois o valor padrão é setado para 2, ou seja, kstrosfl espera dois erros no link antes de trocar para o próximo.
    
    caminho_iproute -> esta variável deve receber um valor do tipo string representando o caminho até o arquivo rt_tables, essa variável possui o caminho da instalação padrão, dessa forma, somente altere o valor caso sua instalação do iproute não seja a padrão.
    
    + O arquivo opcoes.links.conf -> Nesse arquivo ficam armazenadas as configurações específicas de cada link, os links devem ser informados linha a linha e devem ser divididos pelo caractere especial ":".
    
    Cada linha deverá ter o formato abaixo:
    
    nome:interface:gateway
    
    nome -> deverá armazenar uma string com tamanho máximo de 5 caracteres e serve para identificar o link a ser usado, o valor informado deverá ser único no arquivo;
    
    interface -> deverá armazenar uma string com tamanho máximo de 5 caracteres e serve para identificar a interface física em que o link estará ligado, o valor informado deverá ser único no arquivo;
    
    gateway -> deverá armazenar uma string com tamanho máximo 15 caracteres e serve para representar o endereço IP que será usado como gateway para o link.
    
    + O arquivo kstrosfl -> este arquivo é o executável da ferramenta e por padrão le já é carregado no modo daemon, ou seja, sempre será executado em segundo plano.
    
    Este binário deverá possuir permissão de leitura no caminho especificado na variável caminho_iproute, bem como em todos os arquivos que ficam dentro da pasta /etc/kstrosfl, também deverá ter permissão de escrita no diratório /opt/kstrosfl pois é nesse diretório que kstrosfl faz o proceidmento de backup e restore do arquivo rt_tables original.
    
    + A script kstrosfl -> Este script é responsável por iniciar e parar o funcionamento de kstrosfl, possuindo todas as entradas padrõe
s, como start, stop, restart e status.
    
3 - Funcionamento

4 - Instalação

5 - Configuração
