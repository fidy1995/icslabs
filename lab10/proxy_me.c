/*
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Jiang Jiheng, 5130379033
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"

sem_t semlog;
sem_t semdns;
FILE* logfile;

struct cinfo_t {
	int fd;
	struct sockaddr_in sockaddr;	
} cinfo_t;

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);

int popen_clientfd(char *hostname, int port) 
{
	int clientfd;
	struct hostent *hp;
	struct sockaddr_in serveraddr;

	if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1; /* check errno for cause of error */

	/* Fill in the server's IP address and port */
	P(&semdns);
	if (!(hp = gethostbyname(hostname))) {
		V(&semdns);
      		return -2; /* check h_errno for cause of error */
	} 
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
	V(&semdns);
	serveraddr.sin_port = htons(port);

	/* Establish a connection with the server */
	if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
		return -1;
	return clientfd;
}

int pOpen_clientfd(char *hostname, int port) 
{
	int rc = popen_clientfd(hostname, port);
	if (rc == -1)
		unix_error("Open_clientfd_s Unix error");
	else if (rc == -2)        
		dns_error("Open_clientfd_s DNS error");
	return rc;
}


void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char buf[MAXLINE], body[MAXBUF];
	
	// build the HTTP response body
	sprintf(body, "<html><title>Proxy Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Proxy Server</em>\r\n", body);
	
	// print the HTTP response
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
	Rio_writen(fd, buf, strlen(buf));
	Rio_writen(fd, body, strlen(body));
}

void doit(int fd, struct sockaddr_in *sockaddr)
{
	rio_t rio;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char host[MAXLINE], path[MAXLINE];
	int port;
	/*char filename[MAXLINE], cgiargs[MAXLINE];
	int is_static;
	struct stat sbuf;*/

	// read request line and headers
	Rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, buf, MAXLINE);
	if (sscanf(buf, "%s, %s, %s", method, uri, version) < 3) {
		clienterror(fd, method, "400", "Bad request", "request line parse error");
		return;
	}
	if (strcasecmp(method, "GET")) {
		clienterror(fd, method, "501", "Not Implemented" ,"proxy does not implement this method");
		return;
	}
	//read_requesthdrs(&rio);
	
	// parse URI from GET request
	/*is static = parse_uri(uri, filename, cgiargs);
	if (stat(filename, &sbuf) < 0) {
		clienterror(fd, method, "404", "Not found", "proxy couldn't find this file")
		return;
	}
	
	if (is_static) { // serve static content
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
			clienterror(fd, method, "403", "Forebidden", "proxy couldn't read this file")
			return;
		}
		serve_static(fd, filename, sbuf.st_size);
	}
	else {
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
			clienterror(fd, method, "403", "Forebidden", "proxy couldn't read this file")
			return;
		}
		serve_dynamic(fd, filename, sbuf.st_size);
	}*/
	
	if (!parse_uri(uri, host, path, &port)) {
		clienterror(fd, method, "400", "Bad request", "uri parse error");
		return;
	}

	sscanf(buf, "%s %s %s\r\n", method, path, version);

	int clientfd = pOpen_clientfd(host, port);
	Rio_readinitb(&rio, clientfd);
	Rio_writen(clientfd, buf, strlen(buf));

	while (Rio_readlineb(&rio, buf, MAXLINE) > 2) 
		Rio_writen(clientfd, buf, strlen(buf));
	Rio_writen(clientfd, "\r\n", 2);
	
	int csize = 0, size;
	while((size = Rio_readnb(&rio, buf, MAXLINE)) != 0) {
		csize += size;
		Rio_writen(fd, buf, size);
	}

	format_log_entry(buf, sockaddr, uri, csize);
	P(&semlog);
	fprintf(logfile, "%s\n", buf);
	fflush(logfile);
	V(&semlog);

	Close(clientfd);
}

void *stwork(void *cinfo)
{
	struct cinfo_t *temp = (struct cinfo_t *) cinfo;
	int connfd = temp->fd;
	struct sockaddr_in sockaddr = temp->sockaddr;
	Pthread_detach(pthread_self());
	doit(connfd, &sockaddr);
	free(cinfo);
	Close(connfd);
	return NULL;
}

/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
	int listenfd, connfd, port, clientlen;
	struct sockaddr_in addr;
	pthread_t tid;

	/* Check arguments */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
		exit(0);
	}

	port = atoi(argv[1]);
	signal(SIGPIPE, SIG_IGN);
	sem_init(&semlog, 0, 1);
 	sem_init(&semdns, 0, 1);
	logfile = fopen("proxy.log", "a");

	listenfd = Open_listenfd(port);
	do {
		clientlen = sizeof(addr);
		connfd = Accept(listenfd, (SA *)&addr, &clientlen);
		struct cinfo_t *cinfo = (struct cinfo_t *)malloc(sizeof(struct cinfo_t));
		cinfo->fd = connfd;
		cinfo->sockaddr = addr;
		Pthread_create(&tid, 0, stwork, (void *)cinfo);
	} while (1);

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
	int len = strlen(uri);
	int porttemp = 80;

	if (strncasecmp(uri, "http://", 7) != 0) 
		return 0;

	char *hostbegin = uri + 7;
	char *pathbegin = strchr(hostbegin, '/');
	if (pathbegin) {
		int pathlen = len - (pathbegin - uri);
		strncpy(pathname, pathbegin, pathlen);
		pathname[pathlen] = 0;
	}
	else {
		strcpy(pathname, "/");
		pathbegin = uri + len;
	}

	char *portbegin = strchr(hostbegin, ':');
	if (portbegin)
		porttemp = atoi(portbegin + 1);
	else
		portbegin = pathbegin;

	int hostlen = pathbegin - hostbegin;
	strncpy(hostname, hostbegin, hostlen);
	hostname[hostlen] = 0;
      
	*port = porttemp;
	return 1;

  /*char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
        hostname[0] = '\0';
        return -1;
    }

    // Extract the host name 
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    // Extract the port number 
    *port = 80; // default 
    if (*hostend == ':')   
        *port = atoi(hostend + 1);

    // Extract the path 
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
        pathname[0] = '\0';
    }
    else {
        pathbegin++;	
        strcpy(pathname, pathbegin);
    }

    return 0;*/
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
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d\n", time_str, a, b, c, d, uri, size);
}


