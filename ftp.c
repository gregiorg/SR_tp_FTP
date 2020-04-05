/*
 * ftp - read a file name and write the content
 */

#include "csapp.h"

void getCmdServer(int connfd, char** cmd);

void actionCmdServer(int connfd, char** cmd);

void putCmdServer(int connfd, char** cmd);

void ftp(int connfd)
{
  size_t n;
  char rawCmd[MAXBUF];
  rio_t rio;
  Rio_readinitb(&rio, connfd);

  while((n = Rio_readlineb(&rio, rawCmd, MAXBUF))) { // read the current user command
    if (n > 1) { // n <= 1 would mean no actual ASCII symbol has been transmited

      char** cmd = splitCmd(rawCmd);   // split raw cmd into tokens

      if (!strcmp("get", cmd[0])) { // user asked for a file transfer from the server
        getCmdServer(connfd, cmd);

      }else if(!(strcmp("cd", cmd[0]) && strcmp("mkdir", cmd[0]) && strcmp("rm", cmd[0]))){
        actionCmdServer(connfd, cmd);

      } else if (!strcmp("put", cmd[0])) { // user asked for a file transfer to the server
        putCmdServer(connfd, cmd);
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

void actionCmdServer(int connfd, char** cmd){
  if (!fork()){
    execvp(cmd[0], cmd);
  }
}
/*
* Function receives a file sent from the client.
* Protocole is similar to the clients get function :
* receive the file size and read as many bytes as necessary. Read with chunks of MAXBUF bytes
*/
void putCmdServer(int connfd, char** cmd) {
  char data[MAXBUF]; // buffer to read the file

  // read the file size
  recv(connfd, data, MAXLINE, 0);
  long int fileSize = atoi(data);

  if(fileSize >= 0) { // means all is good client side
    size_t nbBytesRead; // stores how many bytes are read at each iteration
    int nbBytesRemaning = fileSize; // stores how many bytes remain to be read
    FILE* fd = fopen("test3.txt", "w"); // create the new file to copy into

    while (nbBytesRemaning > 0) { // iterate until the whole file is read

      nbBytesRead = recv(connfd, data, (MAXBUF < nbBytesRemaning ? MAXBUF : nbBytesRemaning), 0); // read the correct amount of data
      nbBytesRemaning -= nbBytesRead; // update how many bytes remain to be read

      fwrite(data, 1, nbBytesRead, fd); // writes the current data chunk into the file
    }
    fclose(fd);
  }
}
