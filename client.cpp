// Name: Muhammad Waleed Malik
// PSID: 2039264
// Course: COSC 3360
// Date: November 3, 2021

//********************************************
//**** MWalMalik Programming Assignment 2 ****
//********************************************

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <math.h>
#include <fstream>
#include <vector>
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;


// error struct used for sockets
void error(char *msg) {
    perror(msg);
    exit(0);
}

// struct for grandchild thread
struct grandchildThreadData {
    struct hostent* GCTDserver;     // stores the server information
    int GCTDport;                   // stores the port information
    int GCTDdigit;                  // stores the single digit value
    bool* GCTDcode = new bool [7];  // stores the address of Array of Binary Codes
};

// struct for child thread
struct childThreadData {
    long int CTDlineValue;          // stores the line from input file.txt
    int CTDtotalDigits;             // stores the total digits per line from file.txt
    int* CTDdigit = new int;        // stores the single digit value from file.txt
    struct hostent* CTDserver;      // stores the server information
    int CTDport;                    // stores the port information
    bool** CTDptrcode = new bool*;  // stores the address of Array of Binary Codes
};

// thread declaration
void* childThread(void*);           // child thread
void* grandchildThread(void*);      // grandchild thread

// function declaration
int* digitSeparator(long int);      // Separates digits from the number, store them in int Array, returns address of Array head


// main()
int main(int argc, char *argv[]) {
    // 1.0 - Input Stage 1 - checking for server and port
    struct hostent* mainServer;
    int mainPort;
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    mainPort = atoi(argv[2]);
    mainServer = gethostbyname(argv[1]);
    if (mainServer == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }


    // 1.1 - Input Stage 2 - file opening
    vector <long int> lines;
    int in = 0;
    while(cin >> in) {
        lines.push_back(in);
    }
    int totalLines = lines.size();
    long int *valueOnLine = new long int[totalLines];
    for(int i = 0; i < totalLines; i++) {
        valueOnLine[i] = lines.at(i);
    }


    // 2.0 - Processing Stage 1 - Child Thread
    pthread_t* child = new pthread_t[totalLines];                   // creating as many child threads as total lines in file.txt
    childThreadData* childArg = new childThreadData[totalLines];    // to pass arguments to child thread

    // 2.1 - Processing Stage 2 - Creating Child Threads
    for (int i = 0; i < totalLines; i++) {
        childArg[i].CTDlineValue = valueOnLine[i];
        childArg[i].CTDserver = mainServer;
        childArg[i].CTDport = mainPort;
        if (pthread_create(&child[i], nullptr, childThread, &childArg[i])) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }

    // 2.2 - Processing Stage 3 - Joining Child Threads
    for (int i = 0; i < totalLines; i++) {
        if (pthread_join(child[i], nullptr)) {
            fprintf(stderr, "Error joining thread\n");
            return 2;
        }
    }


    // 3.0 - Output
    for (int x=0; x<totalLines; x++) {
        for (int y = 0; y < childArg[x].CTDtotalDigits; y++) {
            cout << childArg[x].CTDdigit[y] << " = ";
            for (int z = 0; z < 7; z++) {
                cout << *((childArg[x].CTDptrcode[y])+z);
                if (z != 7 - 1)
                    cout << " ";
        }
        cout << endl;
    }
        if (x != totalLines - 1)
                    cout << endl;
    }


    // 4.0 - Clear Memory
    delete[] valueOnLine;
    delete[] child;
    delete[] childArg;


    return 0;
}


// thread defination
void* childThread(void* i) {
    struct childThreadData* CTptr = (struct childThreadData*)i;
    
    // 1.0 - Processing Stage 1
    int totalDigits = log10((float)CTptr->CTDlineValue) + 1;    // Calculates the size of CTptr->CTDlineValue
    CTptr->CTDdigit = digitSeparator(CTptr->CTDlineValue);     // Stores the address of Array of digits of CTptr->CTDlineValue
    CTptr->CTDtotalDigits = totalDigits;                        // return call for total digits per line

    // 1.1 - Processing Stage 2 - Grandchild Threads
    pthread_t* grandchild = new pthread_t[totalDigits];
    grandchildThreadData* grandchildArg = new grandchildThreadData[totalDigits];

    // 1.2 - Processing Stage 2 - Creating Grandchild Threads
    for (int i = 0; i < totalDigits; i++) {
        grandchildArg[i].GCTDdigit = CTptr->CTDdigit[i];
        grandchildArg[i].GCTDserver = CTptr->CTDserver;
        grandchildArg[i].GCTDport = CTptr->CTDport;
        if (pthread_create(&grandchild[i], nullptr, grandchildThread, &grandchildArg[i])) {
            fprintf(stderr, "Error creating thread\n");
        }
    }

    // 1.3 - Processing Stage 3 - Joining Grandchild Threads
    for (int i = 0; i < totalDigits; i++) {
        if (pthread_join(grandchild[i], nullptr)) {
            fprintf(stderr, "Error joining thread\n");
        }
    }

    // 1.4 - Processing Stage 4 - Returning information to Parent Threads
    for (int i = 0; i < totalDigits; i++) {
        for (int j = 0; j < 7; j++) {
            CTptr->CTDptrcode[i] = &grandchildArg[i].GCTDcode[0];
        }
    }

    // 2.0 - Clear Memory

    return nullptr;
}

void* grandchildThread(void* i) {
    struct grandchildThreadData* GCTptr = (struct grandchildThreadData*)i;

    // 1.0 - Socket Creation
    int sockFD, n;
    struct sockaddr_in serv_addr;
    sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)GCTptr->GCTDserver->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         GCTptr->GCTDserver->h_length);
    serv_addr.sin_port = htons(GCTptr->GCTDport);
    if (connect(sockFD,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    // 2.0 - Write
    n = write(sockFD,&GCTptr->GCTDdigit,sizeof(int));
    if (n < 0) 
        error("ERROR writing to socket");

    // 3.0 - read
    n = read(sockFD,&GCTptr->GCTDcode[0], sizeof(bool[7]));
    if (n < 0) 
        error("ERROR reading from socket");
    
    // 4.0 - closing sockets
    close(sockFD);

    return nullptr;
}


// function defination
int* digitSeparator(long int number) {
    int totalDigits = log10((float)number) + 1;
    int* separatorPtr = new int[totalDigits];
    int j = 0;

    for (int i = totalDigits - 1; i >= 0; i--) {
        long int divisor = pow((float)10, i);
        long int digit = number / divisor;
        number -= digit * divisor;
        separatorPtr[j] = digit;
        j++;
    }

    return separatorPtr;
}