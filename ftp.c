/*
 * ftp - read a file name and write the content
 */

#include "csapp.h"

void getCmdServer(int connfd, char** cmd);
void infoCmdServer(int connfd, char** cmd);

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

      } else if (!strcmp("ls", cmd[0]) | !strcmp("pwd", cmd[0])) {
        infoCmdServer(connfd, cmd);
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

/*
* Function follows the protocole for commands that are asking for information from the server.
* server executes the user command and send the output to the client.
*/
void infoCmdServer(int connfd, char** cmd){
  // pipe to communicate between the child and the parent
  int p[2];
  pipe(p);

  // if (!strcmp("pwd", cmd[0])) {
  //   char data[MAXBUF];
  //   read(p[0], data, MAXBUF);
  //   printf("%s\n", data);
  // }

  /* child process executes the command and stores the result in the pipe
     parent reads in the pipe and sends data to the client */
  if(!Fork()) { // child process
    close(p[0]); // close the pipes output
    Dup2(p[1], 1); // replace standard output with pipes input
    execvp(cmd[0], cmd); // execute user command

  } else { // parent process
    char data[MAXBUF]; // buffer to send the data in
    size_t nbBytesRead;

    close(p[1]); // close pipes input
    nbBytesRead = read(p[0], data, MAXBUF); // read from the pipes output
    close(p[0]); // close pipes output
    send(connfd, data, nbBytesRead, 0); // send data to the client

    printf("server just sent : \n");
    printf("%s\n", data); // ICI MONSIEUR !!!!!!!!!!!!!!!!
    /* TODO : there seems to be data in the pipe even after the read.
       need to flush the pipe */
  }
}
