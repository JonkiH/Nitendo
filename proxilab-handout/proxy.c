/*
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Andrew Carnegie, ac00@cs.cmu.edu 
 *     Harry Q. Bovik, bovik@cs.cmu.edu
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
    char buf[MAXLINE];              // buf 
    char subpath[MAXLINE];          // response from server
    char server[MAXLINE];           // server name
    char method[MAXLINE];           // holds POST / GET method
    char uri[MAXLINE];              // Uniform resourse indendifier
    char version[MAXLINE];          // http prodacal version
    int Content_length;             // length in bites
    char Content_type[MAXLINE];     // type of content
    char Content_encoding[MAXLINE]; // stores type of encodin
    char Transfer_encoding[MAXLINE];// Sore transfer encoding
    rio_t rio;                      // rio_t struct
    SA *addr;                       // client address
}head;

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void echo(int connfd, struct sockaddr_in *addr);
void doit(head *client);
void read_requesthdrs(rio_t *rp);
void serve_static(int fd, char *filename, int filesize);
void server_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *chortmsg, char *longmsg);
void get_filetype(char *filename, char *filetype);
void get_head(head *hed);

/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
    int listenfd;
    head *conhead = Malloc(sizeof(head));
    int port;
    socklen_t clientlen;
    /* Check arguments */
    Signal(SIGPIPE, SIG_IGN);
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }
    
    port = atoi(argv[1]);
    listenfd = Open_listenfd(port);

    while (1){
        clientlen = sizeof(conhead->addr);            
        conhead->fd = Accept(listenfd, conhead->addr, &clientlen);
        doit(conhead);
        Close(conhead->fd); 
    }
    Close(listenfd);
    Free(conhead);
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
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

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
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d", time_str, a, b, c, d, uri, size);
    printf("%s\n", logstring);
}

/* 
 *
 */
void echo(int connfd,  struct sockaddr_in *addr){
    
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", n);
        Rio_writen(connfd, buf, n);
    }
    
}

void doit(head *client) {

    size_t n_bytes;                 // Holds number of bytes
    char buf[MAXLINE];              // Buffer
    head *host = Malloc(sizeof(head));
    /* Initilize the input/output data stream */
    Rio_readinitb(&client->rio, client->fd);

    /* Read the request from client */
    Rio_readlineb(&client->rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", client->method, client->uri, client->version);

    /* If method is not GET, send error */
    if (strcasecmp(client->method, "GET")) { // returns 0 if equals GET
        clienterror(client->fd, client->method, "501", "Not Implemented", "Unsupported method");
        return;
    }
    /* Else process GET method */
    else { 
        if ( parse_uri(client->uri, client->server, client->subpath, &client->port) < 0) {
            printf("There is an problem whit url\n");
            return;
        }
        host->fd = Open_clientfd(client->server, client->port);
        Rio_readinitb(&host->rio, host->fd);
        sprintf(host->buf, "HEAD http://%s/%s HTTP/1.0\r\n", client->server, client->subpath);
          
   //      printf("%s", host->buf );
 		// printf("%s", buf);       
        Rio_writen(host->fd, buf, strlen(buf));

        int size = 0;
        while(strcmp(buf, "\r\n") && ((n_bytes = Rio_readlineb(&client->rio, buf, MAXLINE)) != 0)) {
//            printf("%s", buf );
            Rio_writen(host->fd, buf, strlen(buf));
            size += n_bytes;
        }
        /* Read the response */
        get_head(host);
        if (strstr(host->Transfer_encoding, "chunked"))
        {
            printf("%s", buf);
            printf("%s",host->buf );
            Rio_readlineb(&host->rio, buf, MAXLINE);
        }
        else{
            Rio_readnb(&host->rio, buf, host->Content_length);
            Rio_writen(client->fd, buf, host->Content_length);
    //        Read(host->fd, host->buf, host->Content_length);
            
        }
        Close(host->fd);
    }

    Free(host);
}

/*
 *  clienterror
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char * longmsg){
    char buf[MAXLINE];
    char body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

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
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
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
    sprintf(buf, "Server: Tiny Web Server\r\n");
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
 

void get_head(head *hed){
    char buf[MAXLINE];
    strcpy(hed->buf, "");
    strcpy(hed->Transfer_encoding, "");
    while((Rio_readlineb(&hed->rio, buf, MAXLINE) != 0) && strcmp(buf, "\r\n")){
        sprintf(hed->buf, "%s%s",hed->buf, buf);
        if (strncmp(buf, "Content-Leng", 12) == 0)        {
            sscanf(buf, "Content-Length:  %d", &hed->Content_length);
        }
        if (strncmp(buf, "Content-type", 12) == 0)        {
            sscanf(buf, "Content-type:  %s", hed->Content_type);
        }
        if (strncmp(buf, "Content-Encoding", 12) == 0)        {
            sscanf(buf, "Content-Encoding:  %s", hed->Content_encoding);
        }
        if (strncmp(buf, "Transfer-Encoding:", 12) == 0)        {
            sscanf(buf, "Transfer-Encoding: %s", hed->Transfer_encoding);
        }
    }

    printf("DONE\n");
}
