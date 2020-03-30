/*
 * ftp - read a file name and write the content
 */

#include "csapp.h"


void ftp(int connfd)
{
  size_t n;
  char buf[MAXBUF];
  rio_t rio;
  Rio_readinitb(&rio, connfd);

  n = Rio_readlineb(&rio, buf, MAXBUF);

  if (n > 1) { // n <= 1 would mean no actual ASCII symbol has been transmited

    char** cmd = splitCmd(buf);   // split into tokens

    if (!strcmp("get", cmd[0])) {
      FILE* fd;
      if((fd = fopen(cmd[1], "r"))) { // opened successfully

        int nbBytesRead;
        while((nbBytesRead = fread(buf, 1, MAXBUF, fd))) { // read MAXBUF bytes for data segmenting
          Rio_writen(connfd, buf, nbBytesRead); // send to client
        }
      }
    }
  }
}
