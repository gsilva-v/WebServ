# WebServ

# Introdução
O projeto propõe a criação de um servidor web, com um  funcionamento parecido com o nginx. O programa deve lidar com requisições de clientes, com os métodos GET, POST e DELETE, deve aceitar o upload de arquivos e executar CGIs em alguma linguagem que não necessariamente c++.
# 
# HTTP

## Como funciona um servidor HTTP

## Primeiro, o que é HTTP?
HTTP é um protocolo de transferência que possibilita que as pessoas que inserem a URL do seu site na Web possam ver os conteúdos e dados que nele existem. A sigla vem do inglês Hypertext Transfer Protocol.

Esse sistema é a base da comunicação que existe em toda a internet em que os sites e conteúdos tragam hiperlinks possam ser encontrado mais facilmente pelo publico.

## O funcionamento

HTTP é um protocolo baseado em texto sem conexão. Isso significa que as pessoas que acessam o site da sua empresa enviam solicitações a servidores que exibem em formato de texto, imagens e outros tipos de mídia. Depois que a solicitação é atenddida pelo servidor, a conexão entre o clente e servidor é fechada.

Cada solicitação necessita de uma nova conexão, então cada vez que alguém acessa seu site, o servidor recebe uma nova conexão com uma nova solicitação.

### O que acontece por baixo dos panos:
O navegador se conecta ao servidor e envia uma solicitação HTTP para a pagina desejada. Então, com essa solicitação ja dentro do servidor, ele verifica se aquela pagina existe dentro dele, se é permitido enviar aquela informação, se todo o corpo da requisição está correto, e caso alguma coisa falhe, uma pagina de erro é retornada ao navegador. Após o navegador receber uma resposta do servidor,ele traduz todo aquele texto recebido para a pagina que vemos e fecha a conexão com o servidor.
#
# Sobre o projeto
Esse projeto foi configurado para aceitar essas requisições, e a execução de CGIs está limitada a python e perl. Os arquivos de configuração do servido é bem parecido com o do nginx, com algumas limitacões, claro.
Um exemplo de como criar um arquivo de configuração valido esta aqui(colocar o redirect).

## O funcionamento
O servidor primeiro é configurado com base em um arquivo `.conf`, que tem o ip, porta e as rotas que aquele endereço tem acesso dentro dos arquivos do servidor. Nesse arquivo também contém as paginas de erro, a raiz do site e o tamanho maximo do body size que o cliente pode enviar. No mesmo arquivo de configuração pode existir mais de uma configuração para sites, a única observação quanto a isso é garantir que as portas usadas entre eles são diferentes.


# links
oq é http https://rockcontent.com/br/blog/http/

