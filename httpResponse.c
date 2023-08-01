typedef struct
{
    char *message;
    int code;
    char *type;
} HttpResponse;

HttpResponse *createHttpResponse(char *message, int code, char *type)
{
    HttpResponse *response = (HttpResponse *)malloc(sizeof(HttpResponse));
    response->type = (char *)malloc(128 * sizeof(char));
    response->message = (char *)malloc(1024 * sizeof(char));

    response->code = code;
    strcpy(response->type, type);
    strcpy(response->message, message);

    return response;
}

void freeHttpResponse(HttpResponse *httpResponse)
{
    free(httpResponse->message);
    free(httpResponse->type);
    free(httpResponse);
    httpResponse = NULL;
}

char *httpResponseToText(HttpResponse *httpResponse)
{
    char *response = (char *)malloc(2048 * sizeof(char));

    sprintf(response, "HTTP/1.1 %d %s\r\n"
                      "Content-Type: text/plain\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Access-Control-Allow-Methods: GET, POST\r\n"
                      "Access-Control-Allow-Headers: Content-Type\r\n"
                      "Conteant-Length: %ld\r\n\r\n%s",
            httpResponse->code, httpResponse->type, strlen(httpResponse->message), httpResponse->message);

    return response;
}