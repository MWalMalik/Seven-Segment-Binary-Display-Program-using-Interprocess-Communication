// Code taken from BB example
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
using namespace std;

void fireman(int) {
    waitpid(-1, NULL, WNOHANG);
}

void error(char *msg) {
    perror(msg);
    exit(1);
}

// function to modify and pass the array to client
void check(bool[], int);  


// main()
int main(int argc, char *argv[]) {
    // Socket creation under Parent and checking Input
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
    listen(sockfd,100);
    clilen = sizeof(cli_addr);
     
    int digit;
    signal(SIGCHLD, fireman);   //kills jombie process

    while(1) { // while start
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");

        // child process
        if (fork() == 0) {
            // READ
            n = read(newsockfd, &digit, sizeof(int));
            if (n < 0) error("ERROR reading from socket");

            // WRITE
            bool sendCode[7];
            check(sendCode, digit); // modifying the array
            n = write(newsockfd,&sendCode, sizeof(bool[7]));    // passing the array to client
            if (n < 0) error("ERROR writing to socket");

            // Closing the socket
            close(newsockfd);
            _exit(0);   // exiting the child process
        }
    }

    close(sockfd);  // exiting the parent process
     return 0; 
}


// Function defination
void check(bool list[], int value) {
    bool codes[10][7] = { {1, 1, 1, 1, 1, 1, 0}, {0, 1, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 1, 0, 1}, {1, 1, 1, 1, 0, 0, 1},
                            {0, 1, 1, 0, 0, 1, 1}, {1, 0, 1, 1, 0, 1, 1}, {1, 0, 1, 1, 1, 1, 1}, {1, 1, 1, 0, 0, 0, 0},
                            {1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 0, 1, 1} };

    for (int i = 0; i < 7; i++) {
        list[i] = codes[value][i];  // Storing the row of 2-D in 1-D array
    }
}