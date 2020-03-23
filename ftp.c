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
  printf("server received %u bytes\n", (unsigned int)n);

  if (n > 1) { // n <= 1 would mean no actual ASCII symbol has been transmited
    int p[2];
    pipe(p);

    if(!Fork()) { // child proccess
      buf[strlen(buf)-1] = '\0';
      close(p[0]); // close output
			Dup2(p[1], 1); // replace standard output to pipe

      // create the reading command and executing it
      char* cmd[3];
      cmd[0] = "cat";
      cmd[1] = buf;
      cmd[2] = NULL;
      execvp(cmd[0], cmd);
      // TODO : need some sort of error management

    } else { // father process
      char resBuf[MAXBUF];
      size_t m;

      close(p[1]); // close input
      m = read(p[0], resBuf, MAXBUF); // read in pipe
      close(p[0]); // close output
      Rio_writen(connfd, resBuf, m); // write to client
    }
  }
}
