/*
 * ftpclient.c - An ftp client
 */
#include <time.h>

#include "csapp.h"

void getCmdClient(int clientfd, char* buf);
void infoCmdClient(int clientfd, char* rawCmd);

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host;
    char rawCmd[MAXLINE];
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = 2121;

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, port);

    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("Connected to %s\n", host);

    Rio_readinitb(&rio, clientfd);

    int isConnectionOpen = 1;

    while(isConnectionOpen) {
      printf("ftp> "); // prompt line
      if(Fgets(rawCmd, MAXLINE, stdin)) { // read user imput
          char** cmd = splitCmd(rawCmd);   // split raw cmd into tokens

          if(!strcmp("get", cmd[0])) {  // get command
              getCmdClient(clientfd, rawCmd);

          } else if(!strcmp("ls", cmd[0]) | !strcmp("pwd", cmd[0])) { // info command
            infoCmdClient(clientfd, rawCmd);

          } else if(!strcmp("bye", cmd[0])) { // client asked to end connection
            Close(clientfd);
            isConnectionOpen = 0;

          } else {
              printf("Unkown command\n");
          }
      }
    }

    exit(0);
}

/*
* This function follows the protocole that copies a file from the Server.
* First the user command is sent to the server, then the client reads the size
* of the file and finally, the client reads the file MAXBUF bytes at a time.
* In cass of error, server will send a negative file size
*/
void getCmdClient(int clientfd, char* rawCmd) {
  char data[MAXBUF]; // buffer to read the file

  clock_t before = clock(); // before timestamp for stats later

  send(clientfd, rawCmd, strlen(rawCmd), 0);  // send raw cmd to server

  // read the file size
  recv(clientfd, data, MAXLINE, 0);
  long int fileSize = atoi(data);

  if(fileSize >= 0) { // file size positive : no server error
    size_t nbBytesRead; // stores how many bytes are read at each iteration
    int nbBytesRemaning = fileSize; // stores how many bytes remain to be read
    FILE* fd = fopen("test2.txt", "w"); // create the new file to copy into

    while (nbBytesRemaning > 0) { // iterate until the whole file is read

      nbBytesRead = recv(clientfd, data, (MAXBUF < nbBytesRemaning ? MAXBUF : nbBytesRemaning), 0); // read the correct amount of data
      nbBytesRemaning -= nbBytesRead; // update how many bytes remain to ne read

      fwrite(data, 1, nbBytesRead, fd); // writes the current data chunk into the file
    }
    fclose(fd);

    clock_t after = clock(); // after timestamp for stats later
    printf("Transfer successfull\n");
    printf("%ld bytes received in %ld milliseconds\n", fileSize, (after - before)); // stats message

  } else { // server sent a negative file size : server error
    printf("Error : server couldn't find file. Please check command\n");
  }
}

/*
* Function follows the protocole for commands that are asking for information from the server.
* client simply sends command and waits for response
*/
void infoCmdClient(int clientfd, char* rawCmd) {
  char data[MAXLINE]; // buffer to read the servers response

  send(clientfd, rawCmd, strlen(rawCmd), 0); // send the user command

  recv(clientfd, data, MAXLINE, 0); // receive the information from the server

  printf("%s\n", data); // display for user
}
