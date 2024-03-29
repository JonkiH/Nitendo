/*
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Jón Heiðar Erlendsson, jone11@ru.is
 *	   Ragnar Ómarsson, ragnaro08@ru.is
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"

/*
 * Struck
 */
typedef struct 
{
    int fd;                         // File descriptor
    int port;                       // Port 
    int Content_length;             // length in bites
    char buf[MAXLINE];              // buf 
    char server[MAXLINE];           // server name
    char subpath[MAXLINE];          // response from server
    char method[MAXLINE];           // holds POST / GET method
    char uri[MAXLINE];              // Uniform resourse indendifier
    char version[MAXLINE];          // http prodacal version
    char Content_type[MAXLINE];     // type of content
    char Content_encoding[MAXLINE]; // stores type of encodin
    char Transfer_encoding[MAXLINE];// Sore transfer encoding
    struct sockaddr_in addr;        // client address
    socklen_t clientlen;            // client addr lenght
    rio_t rio;                      // rio_t struct
}head;

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void echo(int connfd, struct sockaddr_in *addr);
void doit(head client);
void read_requesthdrs(rio_t *rp);
void serve_static(int fd, char *filename, int filesize);
void server_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *chortmsg, char *longmsg);
void get_filetype(char *filename, char *filetype);
void to_file(char *proxy_log);
void *thread(void *vargp);

sem_t mutex;
sem_t mutex_ghn;
sem_t mutex_log;

/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
    int port;
    int listenfd;
    pthread_t tid;
    
    head *conhead;
    /* Check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }
    
    Signal(SIGPIPE, SIG_IGN);
    Sem_init(&mutex_ghn, 0, 1);
    Sem_init(&mutex_log, 0, 1);
    Sem_init(&mutex, 0, 1);
    port = atoi(argv[1]);
    listenfd = Open_listenfd(port);

    while (1){
        conhead = Malloc(sizeof(head));
        conhead->clientlen = sizeof(conhead->addr); 
        conhead->fd = Accept(listenfd, (SA*)&(conhead->addr), &(conhead->clientlen));
        Pthread_create(&tid, NULL, thread, (void*)conhead);
    }
    Close(listenfd);
    exit(0);
}

/*
 * parse_uri - URI parser
 * 
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
    int len;
    char *hostbegin;
    char *hostend;
    char *pathbegin;

    if (strncasecmp(uri, "http://", 7) != 0) {
       hostname[0] = '\0';
       return -1;
    }
       
    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';
    
    /* Extract the port number */
    *port = 80; /* default */
    if (*hostend == ':')   
       *port = atoi(hostend + 1);
    
    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
       pathname[0] = '\0';
    }
    else {
       pathbegin++; 
       strcpy(pathname, pathbegin);
    }
    
    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
              char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;

    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s /%d", time_str, a, b, c, d, uri, size);
    printf("%s\n", logstring);
}

/* 
 *
 */

void doit(head client) {
    
    int size = 0;                   // The size of the data transfer
    size_t n_bytes;                 // Holds number of bytes per read
    char buf[MAXLINE];              // Buffer
    char proxy_log[MAXLINE];        // Log the server traffic
    head host;						// struct for host
    
    /* Initilize the input/output data stream */
    Rio_readinitb(&client.rio, client.fd);
    
    /* Read the request from client */
    if (Rio_readlineb(&client.rio, buf, MAXLINE) == -1){
        Pthread_exit(NULL);
    }
    else {
        sscanf(buf, "%s %s %s", client.method, client.uri, client.version);
        
        /* If method is not GET, send error */
        if (strcasecmp(client.method, "GET")) { // returns 0 if equals GET
            clienterror(client.fd, client.method, "501", "Proxy Server Does Not  Implemented", "Unsupported method");
            return;
        }
        /* Else process GET method */
        else { 
            if (parse_uri(client.uri, client.server, client.subpath, &client.port) < 0){
                clienterror(client.fd, client.method, "501", "Proxy Server URI Error", "Parse Error");
                return;
            }
            
            /* listen to host */
            P(&mutex_ghn);
            host.fd = Open_clientfd(client.server, client.port);
            V(&mutex_ghn);

            /* Initilize i/o and send request */
            Rio_readinitb(&host.rio, host.fd);

            /* Read the response */
            sprintf(buf, "%s /%s %s\r\n", client.method, client.subpath, client.version);
            strcpy(host.buf, buf);    
            while(strcmp(buf, "\r\n") && ((n_bytes = Rio_readlineb(&client.rio, buf, MAXLINE)) != 0)) {
                if(strstr(buf, "chunked")){
                    strcpy(host.Transfer_encoding, "chunked");
                }
                else if(strncmp(buf, "Connection: keep-alive", 18) == 0){
                    sprintf(host.buf, "%s%s", host.buf, "Connection: close\r\n");
                }
                else {
                    sprintf(host.buf, "%s%s", host.buf, buf);       
                }
            }
            Rio_writen(host.fd, host.buf, strlen(host.buf));


            /* Write the response to the client file descriptor 
               http chunked transfer */
            if (strcmp(host.Transfer_encoding, "chunked")){
                while(((n_bytes = Rio_readlineb(&host.rio, buf, MAXLINE)) != 0) && strcmp(buf, "\0\r\r")){
                    Rio_writen(client.fd, buf, n_bytes);
                    size += n_bytes;
                }
            }
            /* http content length transfer */
            else{
            	/* get the content length */
                size = host.Content_length;

                /* if content less the maxline, transfer */
                if(host.Content_length < MAXLINE){
                    Rio_readnb(&host.rio, buf, host.Content_length);
                    Rio_writen(client.fd, buf, host.Content_length); 
                }
                /* else split transfer into segments */
                else if(host.Content_length > MAXLINE){
            		/* Transfer maxline bytes */
                    while(host.Content_length > MAXLINE){
                        n_bytes = Rio_readnb(&host.rio, buf, MAXLINE);
                        Rio_writen(client.fd, buf, n_bytes);
                        /* update the amount left to transfer*/
                        host.Content_length -= n_bytes; 
                    }
                    if(host.Content_length > 0){
                        Rio_readnb(&host.rio, buf, host.Content_length);
                        Rio_writen(client.fd, buf, host.Content_length);
                    }  
                }
            }
            /* Format the proxy log string then write it to a file */
            format_log_entry(proxy_log, &(client.addr), client.uri, size);
            to_file(proxy_log);
            
            /* Close the file descriptor and free memory */
            Close(host.fd);
            Close(client.fd);
        }
    }
}

/*
 *  clienterror
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char * longmsg){
    char buf[MAXLINE];
    char body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Proxy Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Proxy server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}


/*
 * Serve_static
 */
void serve_static(int fd, char *filename, int filesize){
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Proxy Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

/*
 * Server_dynamic
 */
void server_dynamic(int fd, char *filename, char *cgiargs) {
    char buf[MAXLINE], *emptylist[] = {NULL};

    /* Rerurn first part of HTTP response*/
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Proxy Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0){
        /* Reak server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO);
        Execve(filename, emptylist, environ);
    }
    Wait(NULL); /* Parent waits for and reaps child*/
}

void read_requesthdrs(rio_t *rp){

    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")){
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

void get_filetype(char *filename, char *filetype){
    if(strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if(strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if(strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}
 

void to_file(char *proxy_log){

    /* Lock the the thread while writing to file */ 
    P(&mutex_log);

    /* Open data stream to file, append to exsisting file */
    FILE *logFile;
    if((logFile = fopen("proxy.log", "a+")) == NULL){
        /* if unable to open the file send error */
        fprintf(stderr, "501 Proxy Server Error, unable to log\n");
        return;
    }
    /* Print to the file and close data stream */
    fprintf(logFile, "%s\n", proxy_log);
    fclose(logFile);

    /* Unlock the thread */
    V(&mutex_log); 
}

void *thread(void *vargp){

    Pthread_detach(Pthread_self());
    
    head thread = *(head *)vargp;
    Free(vargp);
    doit(thread);
    
    return NULL;
}