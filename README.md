# kstrosfl
Failover de links

1 - Apresentação:

    Esta ferramenta trata sobre o processo de failover em sistemas operacionais GNU/Linux, mais precisamente na distribuição Debian.
    
    Atualmente existem várias ferramentas embarcadas para solucionar este problema em uma rede, ferramentas estas que são conhecidas tecnicamente como "apliance" para o sistema operacional e dependem de um aplicativo externo, uma outra ferramente bem difundida na internet é um script chamado "gwping" com suas variações e pode ser baixado também no repositório GitHub.
    
    Particularmente achei sua implementação muito complicada e por esse motivo, tomei a decisão de desenvolver um aplicativo que tornar-se muito fácil a implementação do processo da failover em uma rede e de forma ao aplicativo trabalhar como um daemon nativo do sistema.
    
    A ferramenta foi desenvolvida sob o codinome "kstrosfl" e possui um monitoramento ativo para até 5 links de dados.

2 - A ferramenta:
    
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
    
    + A script kstrosfl -> Este script é responsável por iniciar e parar o funcionamento de kstrosfl, possuindo todas as entradas padrões, como start, stop, restart e status.
    
3 - Funcionamento:
    
    A lógica de funcionamento de kstrosfl baseia-se em ter um link principal e até quatro links a serem usados como backup.
    
    Sempre será dado priorodade na saída pelo link principal e para um link ser tratado como principal o mesmo deverá ser o primeiro link a ser informado no arquivo opcoes.links.conf.
    
    Todos os links informados após o link principal serão tratados como links de backup e o kstrosfl irá obeder a ordem de inserção, dessa forma, o primeiro link será o link zero, o segundo link será o link um e assim sucessivamente.
    
    A cada loop realizado o kstrosfl executará todos os testes para perceber se os links estão ativos e gravará todas as mensagens no arquivo de log so sistema e para o usuário monitorar o funcionamento basta abrir o arquivo de log conforma abaixo:
    
    # tail -f /var/log/syslog

4 - Instalação:
    
    A ferramenta kstrosfl está compilada como um pacote deb e dessa forma somente foi testada em sistemas GNU/Linux Debian, mas possui um arquivo com a extensão tar para que o mesmo seja instalado manualmente em outras distribuições seguindo a localização dos arquivos especificadas anteriormente.
    
    Para instalação em sistemas GNU/Linux Debian, após realizar o download do arquivo .deb deverá ser instalado usando o comando dpkg, conforme linha de comando abaixo.
    
    # dpkg -i "pacote .deb"
    
    Para instalações da ferramenta diretamente pelos arquivos é necessário um passo adicional que será a instalação do binário kstrosfl como um serviço, no GNU/Linux Debian isso é conseguido pelo comando insserv, mas isso pode mudar conforme sua distribuição.

5 - Configuração:
    
    Para configurar o kstrosfl deverá ser editado somente os arquivos que constam dentro do diretório /etc/kstrosfl
    
    É fortemente recomendado editar somente o arquivo /etc/kstrosfl/opcoes.links.conf pois é justamente o arquivo que armazena os links a serem usados como redundância.
    
    O arquivo /etc/kstrosfl/opcoes.conf deverá ser editado somente caso haja a necessidade de alterar alguma funcionalidade de uso geral da ferramenta, não sendo recomendado por se tratar de valores padrão.
    
    Após finalziar toda a configuração do kstrosfl o mesmo poderá ser iniciado pelo comando abaixo:
    
    # service kstrosfl start
    
    Devido a ferramenta ser instalada como serviço a mesma irá ser incializada juntamente com o servidor
   
6 - Exemplo:

    Imagine o seguinte cenário, a empresa Restaurante Amarelinho possui três liks de internet sendo o primeiro link com velocidade 1Mb da operadora Net, o segundo link com velocidade 256Kb da operadora Oi e o terceiro com velocidade 5Mb da operadora Vivo.
    
    O link da 1Mb possui configuração 172.16.1.0/30, o link de 256Kb possui a configuração 172.16.2.0/30 e o link de 5Mb possui configuração 172.16.3.0/30.
    
    O link de 1Mb está ligado na interface eth0 com o IP 172.16.1.1/30, o link de 256Kb está ligado na interface eth1 com o IP 172.16.2.1/30 e o link de 5Mb está ligado na interface eth2 com IP 172.16.3.1/30.
    
    Analisando o cenário proposto é fortemente recomendado usar o link principal como o link que possui a banda de 5Mb, como primeiro link de backup 1Mb e como segundo link de backup 256Kb.
    
    Para configurar o kstrosfl o arquivo /etc/kstrosfl/opcoes.links.conf deverá ser editado conforme abaixo e configurado seguindo as orientações dadas.
    
    Editando o arquivo:
    
    # vi /etc/kstrosfl/opcoes.links.conf
    
    Configurando o arquivo:
    
    vivo:eth2:172.16.3.2
    net:eth0:172.16.1.2
    oi:172.16.2.2
    
    Após realizar as configurações acima o kstrosfl está devidamente configurado para trabalhar a redundância dos links.
