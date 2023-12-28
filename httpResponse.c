#include <string.h>

typedef struct
{
    char *message;
    int code;
    char *type;
} HttpResponse;

HttpResponse *httpResponse_create(char *message, int code, char *type)
{
    HttpResponse *response = (HttpResponse *)malloc(sizeof(HttpResponse));
    response->type = (char *)malloc((strlen(type) + 1) * sizeof(char));
    response->message = (char *)malloc((strlen(message) + 1) * sizeof(char));

    response->code = code;
    strcpy(response->type, type);
    strcpy(response->message, message);

    return response;
}

void httpResponse_free(HttpResponse *httpResponse)
{
    free(httpResponse->message);
    free(httpResponse->type);
    free(httpResponse);

    httpResponse = NULL;
}

char *httpResponse_to_string(HttpResponse *httpResponse)
{
    int length = strlen(httpResponse->type) + strlen(httpResponse->message) + 3 + 184;
    char *response = (char *)malloc(length * sizeof(char));

    sprintf(response,
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: text/plain\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: %d\r\n\r\n%s",
            httpResponse->code, httpResponse->type, strlen(httpResponse->message), httpResponse->message);

    return response;
}