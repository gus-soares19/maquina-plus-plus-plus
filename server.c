#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "httpResponse.c"
#include "parser.c"

#define PORT 8080

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

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[2048] = {0};
    char response[2048] = {0};
    const char *nomeArquivo = "client.html";

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
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

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
                    sprintf(response, "%s%s", "HTTP/1.1 200 OK\r\n"
                                              "Content-Type: text/html; charset=utf-8\r\n"
                                              "\r\n",
                            conteudo);

                    // Envia a resposta do servidor para o cliente
                    send(new_socket, response, strlen(response), 0);
                    free(conteudo);
                }
                else
                {
                    // requisição inválida
                    write(new_socket, "HTTP/1.1 404 Not Found\n\n", 25);
                }
            }
            // verifica se a requisição é do tipo POST
            else if (strncmp(buffer, "POST", 4) == 0)
            {
                // trata a requisição GET
                char *code = strstr(buffer, "codigo=");
                if (code != NULL)
                {
                    code += 7; // pula "codigo="
                    Parser *parser = (Parser *)malloc(sizeof(Parser));
                    initializeParser(parser);
                    HttpResponse *httpResponse = parse(parser, code);

                    char *text = httpResponseToText(httpResponse);

                    // Envia a resposta do servidor para o cliente
                    write(new_socket, text, strlen(text));
                    free(text);
                    freeHttpResponse(httpResponse);
                    freeParser(parser);
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
