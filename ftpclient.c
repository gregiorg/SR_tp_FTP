/*
 * echoclient.c - An echo client
 */
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
        
        if(!strcmp("get", cmd[0])) {
            // TODO : start timer
            Rio_writen(clientfd, cmd[1], strlen(cmd[1]));
            if(Rio_readlineb(&rio, buf, MAXLINE) > 0) {
                
                // TODO : stop timer
                printf("Transfer successfull\n");
                // TODO : display stats
                
                FILE* fp = fopen("test2.txt", "w"); // create and copy content into knew file
                fwrite(buf, strlen(buf), 1, fp);
                fclose(fp);
            }
        } else {
            printf("Unkown command\n");
        }
        
    }

    Close(clientfd);
    exit(0);
}

char** splitCmd(char* rawCmd) {
    char** resCmd = malloc(2*sizeof(char**));
    char delim[] = " ";
    
    resCmd[0] = strtok(rawCmd, delim);
    resCmd[1] = strtok(NULL, delim);
    
    return resCmd;
}












