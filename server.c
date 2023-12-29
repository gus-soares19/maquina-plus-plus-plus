#include <nuttx/config.h>
#include <netutils/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "httpResponse.c"
#include "parser.c"

#define PORT 8080
#define MAX_CONNECTIONS 20

static int conn_counter;
static bool executing;
static const char *ip;

bool connect_wifi(void)
{
    char *psk_command = "wapi psk wlan0";
    char *essid_command = "wapi essid wlan0";
    int length;

    printf("tentando conectar na internet...\n");

    // monta e envia o comando com a senha
    length = snprintf(NULL, 0, "%s \"%s\" %d", psk_command, CONFIG_EXAMPLES_M3P_WIFI_PSK, 1) + 1;
    char *buffer = (char *)malloc(length * sizeof(char));

    sprintf(buffer, "%s \"%s\" %d", psk_command, CONFIG_EXAMPLES_M3P_WIFI_PSK, 1);
    printf("%s\n", buffer);
    system(buffer);

    // monta e envia o comando com o nome da rede
    length = snprintf(NULL, 0, "%s \"%s\" %d", essid_command, CONFIG_EXAMPLES_M3P_WIFI_ESSID, 1) + 1;
    buffer = (char *)realloc(buffer, length * sizeof(char));

    sprintf(buffer, "%s \"%s\" %d", essid_command, CONFIG_EXAMPLES_M3P_WIFI_ESSID, 1);
    printf("%s\n", buffer);
    if (system(buffer) != 0)
    {
        perror("senha ou nome incorretos.\n");
        return false;
    }

    // conclui e obtem um IP
    printf("renew wlan0\n");
    if (system("renew wlan0") != 0)
    {
        perror("não foi possível conectar.\n");
        return false;
    }

    // exibe o IP recebido
    printf("ifconfig\n");
    if (system("ifconfig") != 0)
    {
        perror("não foi possível obter um IP.\n");
        return false;
    }

    printf("conexão com a internet bem sucedida.\n");
    return true;
}

const char *get_ip_addr(int socket, struct sockaddr_in address, socklen_t addrlen)
{
    static char buffer[INET_ADDRSTRLEN];
    if (getsockname(socket, (struct sockaddr *)&address, &addrlen) == -1)
    {
        perror("Erro ao obter informações locais.");
        close(socket);
        exit(EXIT_FAILURE);
    }

    const char *ip_str = inet_ntop(AF_INET, &(address.sin_addr), buffer, INET_ADDRSTRLEN);

    if (ip_str == NULL)
    {
        perror("Erro ao obter o IP.");
        close(socket);
        exit(EXIT_FAILURE);
    }

    return ip_str;
}

char *get_html_file(const char *file_name)
{
    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Erro ao abrir o arquivo %s.\n", file_name);
        return NULL;
    }

    // Obtém o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    long int length = ftell(file);
    rewind(file);

    // Aloca memória para armazenar o conteúdo do arquivo
    char *content = (char *)malloc((length + 1) * sizeof(char));
    if (content == NULL)
    {
        fprintf(stderr, "Erro ao alocar memória para o conteúdo do arquivo.\n");
        fclose(file);
        return NULL;
    }

    // Lê o conteúdo do arquivo
    size_t bytes = fread(content, sizeof(char), length, file);
    if (bytes != (size_t)length)
    {
        fprintf(stderr, "Erro ao ler o conteúdo do arquivo.\n");
        fclose(file);
        free(content);
        return NULL;
    }

    // Adiciona o caractere nulo ao final da string
    content[length] = '\0';
    fclose(file);

    return content;
}

char *replace_text(char *input, const char *target, const char *replacement)
{
    char *position = strstr(input, target);

    if (position == NULL)
    {
        return strdup(input);
    }

    int original_length = strlen(input);
    int pattern_length = strlen(target);
    int replacement_length = strlen(replacement);
    int new_size = original_length - pattern_length + replacement_length;

    char *result = (char *)malloc((new_size + 1) * sizeof(char));

    if (result == NULL)
    {
        fprintf(stderr, "Erro ao alocar memória.\n");
        exit(EXIT_FAILURE);
    }

    strncpy(result, input, position - input);

    result[position - input] = '\0';

    strcat(result, replacement);
    strcat(result, position + pattern_length);

    return result;
}

void *request_handler(void *arg)
{
    int socket = *((int *)arg);
    char buffer[2048];
    int bytes;
    const char *file = "/data/client.html";
    char *html_response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";

    memset(buffer, 0, sizeof(buffer));
    bytes = read(socket, buffer, sizeof(buffer));

    if (bytes == 0)
    {
        printf("cliente desconectou.\n");
        pthread_exit(NULL);
    }

    printf("requisição recebida.\n");

    // trata requisições GET
    if (strncmp(buffer, "GET", 3) == 0)
    {
        char *html_file = replace_text(get_html_file(file), "SERVER_IP_AD", ip);
        if (html_file == NULL)
        {
            // arquivo de interface não encontrado
            write(socket, "HTTP/1.1 404 Not Found\n\nArquivo de interface não encontrado.", 61);
            close(socket);
            printf("requisição encerrada.\n");

            conn_counter--;
            pthread_exit(NULL);
        }

        char *response = (char *)malloc((strlen(html_file) + strlen(html_response) + 1) * sizeof(char));
        strcpy(response, html_response);
        strcat(response, html_file);

        send(socket, response, strlen(response), 0);
        printf("interface web retornada.\n");

        free(html_file);
        free(response);
    }
    else if (strncmp(buffer, "POST", 4) == 0)
    {
        char *json_text = strstr(buffer, "\r\n\r\n");
        if (json_text == NULL)
        {
            // requisição mal formatada
            write(socket, "HTTP/1.1 400 Bad Request\n\nJSON mal formatado.", 44);
            close(socket);
            printf("JSON mal formatado.\n");

            conn_counter--;
            pthread_exit(NULL);
        }

        json_text += 4;

        cJSON *json;
        json = cJSON_Parse(json_text);

        if (json == NULL)
        {
            // requisição mal formatada
            write(socket, "HTTP/1.1 400 Bad Request\n\nJSON mal formatado.", 44);
            close(socket);
            printf("JSON mal formatado.\n");

            conn_counter--;
            pthread_exit(NULL);
        }

        cJSON *code = cJSON_GetObjectItem(json, "code");
        cJSON *timer = cJSON_GetObjectItem(json, "timer");
        cJSON *delay = cJSON_GetObjectItem(json, "delay");
        cJSON *mode = cJSON_GetObjectItem(json, "mode");

        if (code == NULL || timer == NULL || delay == NULL || mode == NULL)
        {
            // requisição mal formatada
            write(socket, "HTTP/1.1 400 Bad Request\n\nJSON mal formatado.", 44);
            close(socket);
            printf("Erro ao obter valores do JSON.\n");

            conn_counter--;
            pthread_exit(NULL);
        }

        // apenas uma interpretação por vez é permitida
        if (atoi(mode->valuestring) == 1 && executing)
        {
            // requisição ignorada
            write(socket, "HTTP/1.1 429 Too Many Requests\n\nServidor ocupado.", 49);
            close(socket);
            printf("apenas uma interpretação pode ser feita por vez.\n");

            conn_counter--;
            pthread_exit(NULL);
        }

        if (atoi(mode->valuestring) == 1)
        {
            executing = true;
        }

        Parser *parser = (Parser *)malloc(sizeof(Parser));
        parser_init(parser);

        if (parser == NULL)
        {
            write(socket, "500 Interal Server Error\n\nNão foi possível alocar a aplicação.", 61);
            printf("requisição encerrada.\n");
            close(socket);

            if (atoi(mode->valuestring) == 1)
            {
                executing = false;
            }

            perror("falha ao alocar memória para a aplicação.");
            exit(EXIT_FAILURE);
        }

        HttpResponse *httpResponse = parse(parser, code->valuestring, atoi(timer->valuestring), strtod(delay->valuestring, NULL), atoi(mode->valuestring));

        // retorna o resultado da operação
        char *text = httpResponse_to_string(httpResponse);
        write(socket, text, strlen(text));
        printf("%s\n", httpResponse->message);

        if (atoi(mode->valuestring) == 1)
        {
            executing = false;
        }

        free(text);
        httpResponse_free(httpResponse);
        parser_free(parser);
        cJSON_Delete(json);
    }
    else
    {
        // requisição inválida
        write(socket, "HTTP/1.1 405 Method Not Allowed\n\n", 32);
        printf("apenas os métodos GET e POST são aceitos.\n");
    }

    close(socket);
    printf("requisição encerrada.\n");

    conn_counter--;
    pthread_exit(NULL);
}

int main(void)
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    pthread_t tid[MAX_CONNECTIONS];

    conn_counter = 0;

    if (!connect_wifi())
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

    if (listen(server_fd, MAX_CONNECTIONS) < 0)
    {
        perror("falha ao aguardar por requisições.");
        exit(EXIT_FAILURE);
    }

    printf("aguardando requisições na porta %d...\n", PORT);

    // aguarda por requisições
    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("falha ao aceitar requisição.");
            exit(EXIT_FAILURE);
        }

        ip = get_ip_addr(new_socket, address, addrlen);

        if (pthread_create(&tid[conn_counter++], NULL, request_handler, &new_socket) < 0)
        {
            perror("falha ao criar thread para a conexão.");
            exit(EXIT_FAILURE);
        }

        if (conn_counter <= MAX_CONNECTIONS)
        {
            pthread_detach(tid[conn_counter++]);
            conn_counter++;
        }
    }
    return 0;
}
