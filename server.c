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
#define IP "192.168.1.15"

char *lerArquivoHTML(const char *nomeArquivo)
{
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL)
    {
        fprintf(stderr, "Erro ao abrir o arquivo %s\n", nomeArquivo);
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
        fprintf(stderr, "Erro ao alocar memória para o conteúdo do arquivo\n");
        fclose(arquivo);
        return NULL;
    }

    // Lê o conteúdo do arquivo
    size_t lidos = fread(conteudo, sizeof(char), tamanho, arquivo);
    if (lidos != tamanho)
    {
        fprintf(stderr, "Erro ao ler o conteúdo do arquivo\n");
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
    int addrlen = sizeof(address);
    char buffer[2048] = {0};
    char response[4096] = {0};
    const char *nomeArquivo = "/data/client.html";
    bool executing = false;

    // Cria um descritor do socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket falhou");
        exit(EXIT_FAILURE);
    }

    // adiciona o socket na porta informada
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &(address.sin_addr)) < 1) {
        perror("Erro ao converter endereço IP.");
        exit(EXIT_FAILURE);
    }
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind falhou");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("aguardando requisicoes no endereco %s:%d...\n", inet_ntoa(address.sin_addr), PORT);
    // aguarda por requisições
    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        valread = read(new_socket, buffer, 1024);

        if (valread > 0)
        {
            // verifica se a requisição é do tipo GET
            if (strncmp(buffer, "GET", 3) == 0)
            {
                // trata a requisição GET
                char *conteudo = lerArquivoHTML(nomeArquivo);

                if (conteudo != NULL)
                {                    
                    char *html = replaceTextByIP(conteudo, "SERVER_IP_AD", IP);
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
                char *code = getValue(strstr(buffer, "codigo="), "codigo=");
                char *in0 = getValue(strstr(buffer, "in0="), "in0=");
                char *in1 = getValue(strstr(buffer, "in1="), "in1=");
                char *in2 = getValue(strstr(buffer, "in2="), "in2=");
                char *in3 = getValue(strstr(buffer, "in3="), "in3=");

                if (code != NULL && in0 != NULL && in1 != NULL && in2 != NULL && in3 != NULL && timer != NULL)
                {
                    Parser *parser = (Parser *)malloc(sizeof(Parser));
                    initializeParser(parser);

                    HttpResponse *httpResponse = parse(parser, code, in0, in1, in2, in3, timer);

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
