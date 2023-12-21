#include <nuttx/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "httpResponse.c"
#include "parser.c"

#include <unistd.h>

#define PORT 8080

bool conectarInternet(void)
{
  char buffer[100];
  char *psk_command = "wapi psk wlan0";
  char *essid_command = "wapi essid wlan0";

  printf("tentando conectar na internet...\n");

  sprintf(buffer, "%s \"%s\" %d", psk_command, CONFIG_EXAMPLES_M3P_WIFI_PSK, 1);
  printf("%s\n", buffer);
  system(buffer);

  strcpy(buffer, "");

  sprintf(buffer, "%s \"%s\" %d", essid_command, CONFIG_EXAMPLES_M3P_WIFI_ESSID, 1);
  printf("%s\n", buffer);
  if(system(buffer) != 0)
  {
    perror("senha ou nome incorretos.\n");
    return false;
  }

  printf("renew wlan0\n");
  if(system("renew wlan0") != 0)
  {
    perror("nao foi possivel conectar.\n");
    return false;
  }

  printf("ifconfig\n");
  if(system("ifconfig") != 0)
  {
    perror("nao foi possivel obter um IP.\n");
    return false;
  }

  printf("conexao com a internet bem sucedida.\n");
  return true;
}

const char *obterIP(int socket, struct sockaddr_in address, socklen_t addrlen)
{
    static char buffer[INET_ADDRSTRLEN];
    if (getsockname(socket, (struct sockaddr*)&address, &addrlen) == -1) {
        perror("Erro ao obter informações locais.");
        close(socket);
        exit(EXIT_FAILURE);
    }

    const char *ip_str = inet_ntop(AF_INET, &(address.sin_addr), buffer, INET_ADDRSTRLEN);

    if (ip_str == NULL) {
        perror("inet_ntop.");
        close(socket);
        exit(EXIT_FAILURE);
    }

    return ip_str;
}

char *lerArquivoHTML(const char *nomeArquivo)
{
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL)
    {
        fprintf(stderr, "Erro ao abrir o arquivo %s.\n", nomeArquivo);
        return NULL;
    }

    // Obtém o tamanho do arquivo
    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    rewind(arquivo);

    // Aloca memória para armazenar o conteúdo do arquivo
    char *conteudo = (char *)malloc(sizeof(char) * (tamanho + 1));
    if (conteudo == NULL)
    {
        fprintf(stderr, "Erro ao alocar memória para o conteúdo do arquivo.\n");
        fclose(arquivo);
        return NULL;
    }

    // Lê o conteúdo do arquivo
    size_t lidos = fread(conteudo, sizeof(char), tamanho, arquivo);
    if (lidos != tamanho)
    {
        fprintf(stderr, "Erro ao ler o conteúdo do arquivo.\n");
        fclose(arquivo);
        free(conteudo);
        return NULL;
    }

    // Adiciona o caractere nulo ao final da string
    conteudo[tamanho] = '\0';

    fclose(arquivo);

    return conteudo;
}

char *getValue(char *text, char *var)
{
    char *str = strchr(strstr(text, var), '&');

    if (str != NULL)
    {
        size_t tamanho = str - text;
        char *destino = (char *)malloc((tamanho + 1) * sizeof(char));

        if (destino != NULL)
        {
            strncpy(destino, text, tamanho);
            destino[tamanho] = '\0'; // Adiciona o caractere nulo para terminar a string destino
            return destino + strlen(var);
        }
    }

    return text + strlen(var);
}

char *replaceTextByIP(char *input, const char *target, const char *replacement)
{
    char *position = strstr(input, target);

    if (position == NULL) {
        return strdup(input);
    }

    int originalLength = strlen(input);
    int patternLength = strlen(target);
    int replacementLength = strlen(replacement);
    int newSize = originalLength - patternLength + replacementLength;

    char *result = (char *)malloc(newSize + 1);

    if (result == NULL) {
        fprintf(stderr, "Erro ao alocar memória.\n");
        exit(EXIT_FAILURE);
    }

    strncpy(result, input, position - input); 

    result[position - input] = '\0';   

    strcat(result, replacement);
    strcat(result, position + patternLength);

    return result;
}

int main(int argc, char *argv[]) 
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[2048] = {0};
    char response[3072] = {0};
    const char *nomeArquivo = "/data/client.html";
    bool executing = false;

    if(!conectarInternet())
    {
        exit(EXIT_FAILURE);
    }

    // Cria um descritor do socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket falhou.");
        exit(EXIT_FAILURE);
    }

    // adiciona o socket na porta informada
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt.");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind falhou.");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen.");
        exit(EXIT_FAILURE);
    }

    printf("aguardando requisicoes na porta %d...\n", PORT);
    // aguarda por requisições
    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept.");
            exit(EXIT_FAILURE);
        }

        valread = read(new_socket, buffer, 1024);

        if (valread > 0)
        {
            printf("requisicao recebida.\n");

            // verifica se a requisição é do tipo GET
            if (strncmp(buffer, "GET", 3) == 0)
            {
                // trata a requisição GET
                char *conteudo = lerArquivoHTML(nomeArquivo);

                if (conteudo != NULL)
                {                    
                    char *html = replaceTextByIP(conteudo, "SERVER_IP_AD", obterIP(new_socket, address, addrlen));
                    sprintf(response, "%s%s", "HTTP/1.1 200 OK\r\n"
                                              "Content-Type: text/html; charset=utf-8\r\n"
                                              "\r\n",
                            html);

                    // Envia a resposta do servidor para o cliente
                    send(new_socket, response, strlen(response), 0);
                    free(conteudo);
                    free(html);
                }
                else
                {
                    // requisição inválida
                    write(new_socket, "HTTP/1.1 404 Not Found\n\n", 25);
                }
            }
            // apenas uma requisicao sera aceita por vez
            else if (executing)
            {
                write(new_socket, "HTTP/1.1 429 Too Many Requests\n\n", 33);
            }
            // verifica se a requisição é do tipo POST
            else if (strncmp(buffer, "POST", 4) == 0)
            {
                executing = true;

                // trata a requisição POST
                char *timer = getValue(strstr(buffer, "timer="), "timer=");
                char *pausa = getValue(strstr(buffer, "pausa="), "pausa=");
                char *code = getValue(strstr(buffer, "codigo="), "codigo=");

                if (code != NULL && timer != NULL && pausa !=NULL)
                {
                    Parser *parser = (Parser *)malloc(sizeof(Parser));
                    initializeParser(parser);

                    HttpResponse *httpResponse = parse(parser, code, timer, pausa);

                    char *text = httpResponseToText(httpResponse);

                    // Envia a resposta do servidor para o cliente
                    write(new_socket, text, strlen(text));
                    free(text);
                    freeHttpResponse(httpResponse);
                    freeParser(parser);
                    executing = false;
                }
                else
                {
                    // requisição inválida
                    write(new_socket, "HTTP/1.1 400 Bad Request\n\n", 25);
                }
            }
            else
            {
                // requisição inválida
                write(new_socket, "HTTP/1.1 400 Bad Request\n\n", 25);
            }
        }
        
        memset(buffer, 0, sizeof(buffer));
        memset(response, 0, sizeof(response));
        close(new_socket);
    }
    return 0;
}
