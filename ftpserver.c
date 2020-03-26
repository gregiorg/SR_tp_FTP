/*
 * ftpserver.c - An iterative ftp server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define MAX_NB_FILS 2

void ftp(int connfd);

void sigchldHandler(int sig) {
  int status;
  waitpid(-1, &status, WNOHANG|WUNTRACED);
  return;
}

/*
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{
    Signal(SIGCHLD, sigchldHandler);
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(0);
    }
    port = 2121;

    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);


    for(int i = 0; i < MAX_NB_FILS; i++) {
      if(!Fork()) {
        while(1) {
          connfd = Accept(listenfd, (SA*) &clientaddr, &clientlen);

          /* determine the name of the client */
          Getnameinfo((SA *) &clientaddr, clientlen,
                      client_hostname, MAX_NAME_LEN, 0, 0, 0);

          /* determine the textual representation of the client's IP address */
          Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                    INET_ADDRSTRLEN);

          printf("server connected to %s (%s)\n", client_hostname,
                 client_ip_string);

          ftp(connfd);
          Close(connfd);
        }
        exit(0);
      }
    }

    for(int i = 0; i < MAX_NB_FILS; i++) {
      Wait(NULL);
    }

  printf("All children are dead\n");
    exit(0);
}
