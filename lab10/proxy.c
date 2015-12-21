/*
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Jiang Jiheng, 5130379033
 * 
 * A simple web proxy implement.
 * Rewrapped rio_readlineb, rio_readnb and rio_writen.
 * Rewrited and wrapped open_clientfd.
 * Got error report and a part of "doit" function from ics book.
 * Modified parse_uri and format_log_entry.
 * Added a startwork(void *stwork) for multi-thread.
 */ 

#include "csapp.h"
#include <stdarg.h>

#define _DEFAULT 23333

typedef struct cinfo
{
	int connfd;
	struct sockaddr_in sockaddr;
} cinfo_t;

/*
 * Gloval vars
 */
sem_t semlog;
sem_t semdns;
FILE *logfile;

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void *stwork(void *arg);
void doit(int connfd, struct sockaddr_in *sockaddr);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
int open_clientfd_r(char *hostname, int port);
int Open_clientfd_w(char *hostname, int port);

/*
 * wrapper of three rio functions
 */
ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf, size_t n) 
{
	ssize_t rc;

	if ((rc = rio_readnb(rp, usrbuf, n)) < 0) {
		fprintf(stderr, "Rio_readnb error: %s\n", strerror(errno));
		return 0;
	}
	return rc;
}

ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen) 
{
	ssize_t rc;

	if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0) {
		fprintf(stderr, "Rio_readlineb error: %s\n", strerror(errno));
		return 0;
	}
	return rc;
} 

void Rio_writen_w(int fd, void *usrbuf, size_t n) 
{
	if (rio_writen(fd, usrbuf, n) != n)
		fprintf(stderr, "Rio_writen error: %s\n", strerror(errno));
}

/*
 * Open_clientfd_w - wrapper of open_clientfd_s
 */
int Open_clientfd_w(char *hostname, int port) 
{
	int rc = open_clientfd_r(hostname, port);
	if (rc == -1)
		unix_error("Open_clientfd_s Unix error");
	else if (rc == -2)        
		dns_error("Open_clientfd_s DNS error");
	return rc;
}

/*
 * open_clientfd_r - thread-safe version for open_clientfd
 */
int open_clientfd_r(char *hostname, int port) 
{
	int clientfd;
	struct hostent *hp;
	struct sockaddr_in serveraddr;

	if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1; /* check errno for cause of error */

	/* Fill in the server's IP address and port */
	P(&semdns);
	if ((hp = gethostbyname(hostname)) == NULL) {
		V(&semdns);
		return -2; /* check h_errno for cause of error */
	} 
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
	V(&semdns);
	serveraddr.sin_port = htons(port);

 	/* Establish a connection with the server */
	if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
		return -1;
	return clientfd;
}

/* 
 * clienterror - client error report exactly from the book
 */
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


/* 
 * doit - deal with the request by the client
 */
void doit(int connfd, struct sockaddr_in *sockaddr)
{
	// read the request line
	rio_t rio1, rio2;
	char buf[MAXLINE];
	char method[MAXLINE], uri[MAXLINE], ver[MAXLINE];
	/*char filename[MAXLINE], cgiargs[MAXLINE];
	int is_static;
	struct stat sbuf;*/
	
	Rio_readinitb(&rio1, connfd);
	Rio_readlineb_w(&rio1, buf, MAXLINE);
	if(sscanf(buf, "%s %s %s", method, uri, ver) < 3) {
		clienterror(connfd, method, "400", "Bad Request", "request line parse error");
		return;
	}
	if(strcmp(method, "GET")) {
		clienterror(connfd, method, "501", "Not Implemented", "Not Implemented");
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

	// parse uri
	char host[MAXLINE], path[MAXLINE];
	int port;
	if(!parse_uri(uri, host, path, &port)) {
		clienterror(connfd, method, "400", "Bad Request", "uri parse error");
		return;
	}

	// build the request line for real server
	sprintf(buf, "%s %s %s\r\n", method, path, ver);

	// create a connection to real server
	int svrfd = Open_clientfd_w(host, port);
	Rio_readinitb(&rio2, svrfd);

	// send the request line
	Rio_writen_w(svrfd ,buf, strlen(buf));

	// read the request hdr and send it
	char buf2[MAXLINE];
	while(Rio_readlineb_w(&rio1, buf2, MAXLINE) > 2)
		Rio_writen_w(svrfd ,buf2, strlen(buf2));

	// send a empty line
	Rio_writen_w(svrfd, "\r\n", 2);

	// send back response data
	int res_size = 0;
	int size;
	while((size = Rio_readnb_w(&rio2, buf2, MAXLINE)) != 0) {
		res_size += size;
		Rio_writen_w(connfd, buf2, size);
	}

	// write log
	format_log_entry(buf2, sockaddr, uri, res_size);
	P(&semlog);
	fprintf(logfile, "%s\n", buf2);
	fflush(logfile);
	V(&semlog);

	// close
	Close(svrfd);
}

/* 
 * stwork - The function excecuted by sub threads 
 */
void *stwork(void *arg)
{
	cinfo_t *p = (cinfo_t *)arg;
	int connfd = p->connfd;
	struct sockaddr_in sockaddr = p->sockaddr;
	free(arg);
	Pthread_detach(pthread_self());
	doit(connfd, &sockaddr);
	Close(connfd);
	return 0;
}

/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
	int listenfd, connfd, port, clientlen;
	struct sockaddr_in clientaddr;
	pthread_t tid;

    /* Check arguments */
    /*if (argc != 2) {
      fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
      exit(0);
    }*/

	port = (argc != 2)? _DEFAULT: atoi(argv[1]);
	signal(SIGPIPE, SIG_IGN);
	sem_init(&semlog, 0, 1);
	sem_init(&semdns, 0, 1);
	logfile = fopen("proxy.log", "a");

	listenfd = Open_listenfd(port);
    	do{
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		cinfo_t *arg = (cinfo_t *)malloc(sizeof(cinfo_t));
		arg->connfd = connfd;
		arg->sockaddr = clientaddr;
		Pthread_create(&tid, 0, stwork, (void *)arg);
	} while (1);
    
	exit(0);
}


/*
 * parse_uri - URI parser
 * 
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return 0 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
	int len = strlen(uri);
	int tmp_port = 80;

	if (strncasecmp(uri, "http://", 7) != 0) 
		return 0;

	char *hostbegin = uri + 7;
	char *pathbegin = strchr(hostbegin, '/');
	if (pathbegin) {
		int path_len = len - (pathbegin - uri);
		strncpy(pathname, pathbegin, path_len);
		pathname[path_len] = 0;
      
	}
	else {
		strcpy(pathname, "/");
		pathbegin = uri + len;
	}

	char *portbegin = strchr(hostbegin, ':');
	if (portbegin)
		tmp_port = atoi(portbegin + 1);
	else
		portbegin = pathbegin;

	int host_len = pathbegin - hostbegin;
	strncpy(hostname, hostbegin, host_len);
	hostname[host_len] = 0;
      
	*port = tmp_port;
	return 1;

    
    /* Extract the host name */
    /*hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';*/
    
    /* Extract the port number */
    //*port = 80; /* default */
    /*if (*hostend == ':')   
      *port = atoi(hostend + 1);*/
    
    /* Extract the path */
    /*pathbegin = strchr(hostbegin, '/');
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
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d", time_str, a, b, c, d, uri, size);
}
