/*
 * ftp - read a file name and write the content
 */
#include <stdio.h>
#include <stdlib.h>

#include "csapp.h"

void ftp(int connfd)
{
  size_t n;
  char buf[MAXLINE];
  rio_t rio;
  Rio_readinitb(&rio, connfd);

  n = Rio_readlineb(&rio, buf, MAXLINE);

  if (n > 1) { // n <= 1 would mean no actual ASCII symbol has been transmited

    FILE* fd;
    if((fd = fopen(buf, "r"))) { // opened successfully
      // obtain file size
      int fSize;
      fseek(fd, 0, SEEK_END);
      fSize = ftell(fd);
      fseek(fd, 0, SEEK_SET);

      fread(buf, fSize, 1, fd); // read the file in one go

      Rio_writen(connfd, buf, fSize); // send to client

    } else { // error at file opening
      // TODO : check errno
    }
  }
}
