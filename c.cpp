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
using namespace std;

void error(char *msg) {
    perror(msg);
    exit(0);
}

struct code {
    bool col [7];
};

struct info {
    int* digit = new int;
    int* totalDigits = new int;
    code* array = new code;
};

struct grandchildThreadData {
    int GCTDdigit;          // Stores the single digit value
    info* GCTDinfo = new info;
    int GCTDgrandchildIndex;
    int GCTDchildIndex;
    struct hostent* GCTDserver;
    int GCTDport;
    //bool* GCTDptr;
};

struct childThreadData {
    long int CTDlineValue;          // Stores the single digit value
    info* CTDinfo = new info;
    int* CTDdigit = new int;
    int CTDchildIndex;
    struct hostent* CTDserver;
    int CTDport;
};

void* childThread(void*);
void* grandchildThread(void*);
int* digitSeparator(long int);

int main(int argc, char *argv[])
{
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

    // FILE OPENING
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
    // FILE CLOSING

    // CHILD THREAD
    pthread_t* child = new pthread_t[totalLines];
    childThreadData* childArg = new childThreadData[totalLines];
    info* mainInfo = new info[totalLines];

    for (int i = 0; i < totalLines; i++) {
        childArg[i].CTDinfo[i] = mainInfo[i];
        childArg[i].CTDlineValue = valueOnLine[i];
        childArg[i].CTDchildIndex = i;
        childArg[i].CTDserver = mainServer;
        childArg[i].CTDport = mainPort;
        if (pthread_create(&child[i], nullptr, childThread, &childArg[i])) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }
    for (int i = 0; i < totalLines; i++) {
        if (pthread_join(child[i], nullptr)) {
            fprintf(stderr, "Error joining thread\n");
            return 2;
        }
    }
    // CHILD THREAD END

    for (int x=0; x<totalLines; x++) {
        for (int y=0; y<*(mainInfo[x].totalDigits); y++) {
            cout << mainInfo[x].digit[y] << " = ";
            for (int z=0; z<col; z++) {
                cout << mainInfo[x].array[y].col[z];
                if (z != col - 1)
                cout << " ";
            }
            cout << endl;
        }
        if (x != totalLines-1)
            cout << endl;
    }
    

    delete[] valueOnLine;
    delete[] child;
    delete[] childArg;

    return 0;
}

void* childThread(void* i) {
    struct childThreadData* CTptr = (struct childThreadData*)i;
    // run digit seprator
    int totalDigits = log10((float)CTptr->CTDlineValue) + 1;    // Calculates the size of inputNumber
    CTptr->CTDinfo[CTptr->CTDchildIndex].digit = digitSeparator(CTptr->CTDlineValue);     // Stores the address of Array of digits of inputNumber
    CTptr->CTDinfo[CTptr->CTDchildIndex].totalDigits[0] = totalDigits;

    // Grandchildthreading
    pthread_t* grandchild = new pthread_t[totalDigits];
    grandchildThreadData* grandchildArg = new grandchildThreadData[totalDigits];

    // 2.2 - Processing Stage - 3: Creating GranChildthreads
    for (int i = 0; i < totalDigits; i++) {
        grandchildArg[i].GCTDchildIndex = CTptr->CTDchildIndex;
        grandchildArg[i].GCTDgrandchildIndex = i;
        grandchildArg[i].GCTDdigit = CTptr->CTDinfo[CTptr->CTDchildIndex].digit[i];
        CTptr->CTDinfo[CTptr->CTDchildIndex].digit[i] = CTptr->CTDinfo[CTptr->CTDchildIndex].digit[i];
        
        grandchildArg[i].GCTDinfo[CTptr->CTDchildIndex] = CTptr->CTDinfo[CTptr->CTDchildIndex];
        grandchildArg[i].GCTDserver = CTptr->CTDserver;
        grandchildArg[i].GCTDport = CTptr->CTDport;
        if (pthread_create(&grandchild[i], nullptr, grandchildThread, &grandchildArg[i])) {
            fprintf(stderr, "Error creating thread\n");
        }
    }

    // 2.3 - Processing Stage - 4: Joining GrandChildthreads
    for (int i = 0; i < totalDigits; i++) {
        if (pthread_join(grandchild[i], nullptr)) {
            fprintf(stderr, "Error joining thread\n");
        }
    }

    //delete[] grandchild;
    //delete[] grandchildArg;
    //delete[] numberArray;

    return nullptr;
}

void* grandchildThread(void* i) {
    struct grandchildThreadData* GCTptr = (struct grandchildThreadData*)i;

    //GCTptr->GCTDinfo[GCTptr->GCTDchildIndex].digit[GCTptr->GCTDgrandchildIndex]
    //cout << 
    //cout << GCTptr->GCTDdigit;

    int sockFD, n;
    struct sockaddr_in serv_addr;
    //GCTptr->GCTDserver; //server
    //GCTptr->GCTDport; //portno
    
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

// WRITE
    n = write(sockFD,&GCTptr->GCTDinfo[GCTptr->GCTDchildIndex].digit[GCTptr->GCTDgrandchildIndex],sizeof(int));
    if (n < 0) 
         error("ERROR writing to socket");
    
    // READ
    n = read(sockFD,&GCTptr->GCTDinfo[GCTptr->GCTDchildIndex].array[GCTptr->GCTDgrandchildIndex].col, sizeof(bool[7]));
    if (n < 0) 
        error("ERROR reading from socket");

    close(sockFD);
    return nullptr;
}

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