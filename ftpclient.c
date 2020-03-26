/*
 * ftpclient.c - An ftp client
 */
#include <time.h>

#include "csapp.h"

char** splitCmd(char* rawCmd);

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, buf[MAXLINE];
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

    printf("ftp> "); // prompt line
    if(Fgets(buf, MAXLINE, stdin)) {
        char** cmd = splitCmd(buf);   // split into tokens

        if(!strcmp("get", cmd[0])) {  // get command
            clock_t before = clock();
            Rio_writen(clientfd, cmd[1], MAXLINE);  // send file name to server

            size_t nbBytesRead;
            int nbTotalBytesRead = 0;
            if((nbBytesRead = Rio_readnb(&rio, buf, MAXLINE)) > 0) {  // first read. If nothing if read then we have an error
                FILE* fd = fopen("test2.txt", "w"); // create the new to copy into

                do {
                  nbTotalBytesRead += nbBytesRead;  // updating the total number of bytes read for stats later
                  fwrite(buf, nbBytesRead, 1, fd);  // write data into file
                } while((nbBytesRead = Rio_readnb(&rio, buf, MAXLINE)) > 0); // read at max MAXLINE bytes

                fclose(fd);

                clock_t after = clock();
                printf("Transfer successfull\n");
                printf("%d bytes received in %ld milliseconds\n", nbTotalBytesRead, (after - before));

            } else { // server didn't send back data
              printf("Server Error : server didn't send data. Check if command is valid\n");
            }
        } else {
            printf("Unkown command\n");
        }

    }

    Close(clientfd);
    exit(0);
}
/*
 * splitCmd : splits a string input into words and gets rid of spaces and \n
 * input : char* rawCmd - the raw command line to split
 * return : char** - an array of strings
*/
char** splitCmd(char* rawCmd) {
    char** resCmd = malloc(2*sizeof(char**));
    char delim[] = " \n";

    resCmd[0] = strtok(rawCmd, delim);
    resCmd[1] = strtok(NULL, delim);

    return resCmd;
}
