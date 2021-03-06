/*
 * ftpclient.c - An ftp client
 */
#include <time.h>

#include "csapp.h"

void infoCmdClient(int clientfd, char* rawCmd);
void actionCmdClient(int clientfd, rio_t rio, char* rawCmd);
void getCmdClient(int clientfd, char* rawCmd);
void putCmdClient(int clientfd, char* rawCmd, char* fileName);

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

    int isConnectionOpen = 0;

    // login loop
    while(!isConnectionOpen) {
      // buffers
      char login[MAXLINE];
      char password[MAXLINE];
      char servRes[MAXLINE];

      printf("ftp> login : ");
      Fgets(login, MAXLINE, stdin); // read user login
      printf("ftp> password : ");
      Fgets(password, MAXLINE, stdin); // read user password

      // we get rid of the last \n character
      login[strlen(login)-1] = '\0';
      password[strlen(password)-1] = '\0';

      if(strlen(login) && strlen(password)) { // just ignore if user didn't write anything
        // send user inputs to server
        send(clientfd, login, strlen(login), 0);
        send(clientfd, password, strlen(password), 0);

        // receive the connection state from server and convert to int
        recv(clientfd, servRes, MAXLINE, 0);
        isConnectionOpen = atoi(servRes);
      }
    }

    printf("Logged in\n");

    while(isConnectionOpen) {
      printf("ftp> "); // prompt line
      if(Fgets(rawCmd, MAXLINE, stdin)) { // read user imput
        if(strcmp("\n",rawCmd)) {
          char** cmd = splitCmd(rawCmd);   // split raw cmd into tokens
          if(!strcmp("get", cmd[0])) {  // get command
              getCmdClient(clientfd, rawCmd);

          } else if(!strcmp("ls", cmd[0]) | !strcmp("pwd", cmd[0])) { // info command
            infoCmdClient(clientfd, rawCmd);

          } else if(!strcmp("put", cmd[0])) { // put command
            putCmdClient(clientfd, rawCmd, cmd[1]);

          } else if(!strcmp("bye", cmd[0])) { // client asked to end connection
              Close(clientfd);
              isConnectionOpen = 0;
          } else if(!(strcmp("cd", cmd[0]) && strcmp("mkdir", cmd[0]) && strcmp("rm", cmd[0]))){
              actionCmdClient(clientfd, rio, rawCmd);
          } else {
              printf("Unkown command\n");
          }
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

    FILE* fd;
    long int localFileSize;
    if(access(".tmp", F_OK) == 0) { // .tmp exists : the transfer was interupted
      fd = fopen("test2.txt", "a"); // we'll append to last instance of the file
      localFileSize = ftell(fd);
      sprintf(data, "%ld", localFileSize);
      send(clientfd, data, strlen(data), 0); // send local file size

    } else { // tmp doesn't exist : no interupted transfer
      fopen(".tmp", "w"); // marker file that stays if transfer is interupted
      fd = fopen("test2.txt", "w"); // create or overwrite the file the file
      localFileSize = 0;
      send(clientfd, "0", 1, 0); // send local file size
    }
    nbBytesRemaning -= localFileSize; // nothing happens here if transfer wasn't interupted

    while (nbBytesRemaning > 0) { // iterate until the whole file is read
      sleep(1);

      nbBytesRead = recv(clientfd, data, (MAXBUF < nbBytesRemaning ? MAXBUF : nbBytesRemaning), 0); // read the correct amount of data
      nbBytesRemaning -= nbBytesRead; // update how many bytes remain to be read

      fwrite(data, 1, nbBytesRead, fd); // writes the current data chunk into the file
    }
    fclose(fd);
    remove(".tmp");

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

/*
* Function that sends a file to the server.
* Follows a similar protocole as the servers get :
* Send files size and then send file with chunks of MAXBUF bytes
*/
void putCmdClient(int clientfd, char* rawCmd, char* fileName) {
  FILE* fd;
  char data[MAXBUF];

  send(clientfd, rawCmd, strlen(rawCmd), 0);  // send raw cmd to server

  if((fd = fopen(fileName, "r"))) { // opened successfully
    // get file size and send it to server
    fseek(fd, 0, SEEK_END);
    long int fileSize = ftell(fd);
    rewind(fd);
    sprintf(data, "%ld", fileSize);
    send(clientfd, data, strlen(data), 0);

    int nbBytesRead;
    while((nbBytesRead = fread(data, 1, MAXBUF, fd))) { // read MAXBUF bytes for data segmenting
      send(clientfd, data, nbBytesRead, 0); // send to server
    }
    fclose(fd);
    printf("File transfered\n");
  } else { // client error : invalide file name
    printf("Error : can't open file. Check file name or permisions \n");
    send(clientfd, "-1", strlen("-1"), 0); // sending -1 as a file size to server
  }
}

/*
* actionCmdClient is for cmd with no expected return value
*/
void actionCmdClient(int clientfd, rio_t rio, char* rawCmd){
  send(clientfd, rawCmd, strlen(rawCmd), 0);
}
