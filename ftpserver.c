/*
 * ftpserver.c - An iterative ftp server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define MAX_NB_FILS 2

void ftp(int connfd);

typedef struct {
  char* login;
  char* password;
} ftpid_t;

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

          // loop to read all existing IDs
          FILE* fIDs;
          ftpid_t* allIDs = malloc(0);
          int currentIDindex = 0;
          if((fIDs = fopen("IDs.txt", "r"))) { // open file containing all logins and passwords

            char* line;
            size_t n = 0;
            while(getline(&line, &n, fIDs) > 0) { // read all lines of the file. line contains login and password
              allIDs = realloc(allIDs, (currentIDindex + 1) * sizeof(ftpid_t));
              char** splitLine = splitCmd(line);
              allIDs[currentIDindex].login = splitLine[0];
              allIDs[currentIDindex].password = splitLine[1];
              currentIDindex++;
            }
            free(line); // because of getline()

          } else {
            printf("Error : coudn't find ID file\n");
            exit(-1);
          }
          // all IDs are stored in allIDs

          // loop the read the ID sent from the client
          int isConnectionOpen = 0;
          while(!isConnectionOpen) {
            // buffers
            char login[MAXLINE];
            char password[MAXLINE];
            char res[MAXLINE];

            // get login data fromclient
            recv(connfd, login, MAXLINE, 0);
            recv(connfd, password, MAXLINE, 0);

            // compare login data to all stored IDs
            for(int i = 0; i < currentIDindex; i++) {
              if(!strcmp(allIDs[i].login, login) && !strcmp(allIDs[i].password, password)) {
                isConnectionOpen = 1;
              }
            }
            // send connection state to client
            sprintf(res, "%d", isConnectionOpen);
            send(connfd, res, strlen(res), 0);
          }

          ftp(connfd);
          Close(connfd);
        }
        printf("a child just died\n");
        exit(0);
      }
    }

    for(int i = 0; i < MAX_NB_FILS; i++) {
      Wait(NULL);
    }

  printf("All children are dead\n");
    exit(0);
}
