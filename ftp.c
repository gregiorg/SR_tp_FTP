/*
 * ftp - read a file name and write the content
 */

#include "csapp.h"

void getCmdServer(int connfd, char** cmd);

void ftp(int connfd)
{
  size_t n;
  char rawCmd[MAXBUF];
  rio_t rio;
  Rio_readinitb(&rio, connfd);

  while((n = Rio_readlineb(&rio, rawCmd, MAXBUF))) { // read the current user command
    if (n > 1) { // n <= 1 would mean no actual ASCII symbol has been transmited

      char** cmd = splitCmd(rawCmd);   // split raw cmd into tokens

      if (!strcmp("get", cmd[0])) { // user asked for a file transfer
        getCmdServer(connfd, cmd);
      }
    }
  }
}

/*
* Function follows the file transfer protocole from server to client.
* Sends the size of the file then the file itself by chunks of MAXBUF
* In case of an error, sends a negative file size to the client
*/

void getCmdServer(int connfd, char** cmd) {
  FILE* fd;
  char data[MAXBUF];
  if((fd = fopen(cmd[1], "r"))) { // opened successfully

    // get file size and send it to client
    fseek(fd, 0, SEEK_END);
    long int fileSize = ftell(fd);
    rewind(fd);
    sprintf(data, "%ld", fileSize);
    send(connfd, data, strlen(data), 0);

    int nbBytesRead;
    while((nbBytesRead = fread(data, 1, MAXBUF, fd))) { // read MAXBUF bytes for data segmenting
      send(connfd, data, nbBytesRead, 0); // send to client
    }
    fclose(fd);
  } else { // server error : invalide file name
    printf("Invalide file name\n");
    send(connfd, "-1", strlen("-1"), 0); // sending -1 as a file size to client
  }
}
