//
// Author: Capt G. Parks Masters
// Date: 6 Oct 2019
//

#ifndef TCP_MP3SERVER_TCP_FUNCITONS_H
#define TCP_MP3SERVER_TCP_FUNCITONS_H

#define MAXLINE 600

#include "PEX2ClientNew.h"

// Structure is used to track the details of the current connection.
// This data is initialized during TCPConnect (3-way handshake)
// Must be provided to TCPSend/Receive so that subsequent packets have
//   the correct header information, and errors can be handled appropriately.
struct tcp_info{
    int my_seq;         //stores my INITIAL SEQ number established during TCPConect.  This never changes during a connection
    int remote_seq;     //stores server's INITIAL SEQ number established during TCP Connect.  This never changes during a connection
    int data_sent;      //used to determine what my SEQ # and server's ACK # should be
    int data_received;  //used to track what my ACK# and server's SEQ should be
    int remote_data_acknowledged;  //number of bytes server has received.  Updated
};

struct headerStrip{
    int SEQ;
    int ACK;
    char* data;
};

struct headerStrip* stripHeader(char* buffer){
    struct headerStrip *header = malloc(sizeof(struct headerStrip));
    const char* s = "ab234cid*(s349*(20kd";
    int FLAG, SEQ, ACK;
    if (3 == sscanf(buffer,
                    "%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d",
                    &FLAG,
                    &SEQ,
                    &ACK))
    {
        printf("%d %d %d\n", FLAG, SEQ, ACK);
    }
    header->SEQ = SEQ;
    header->ACK = ACK;
    char* pnw = buffer + 34;
    header->data = pnw;
    return header;
}

bool rejectPacket(char* buffer, struct tcp_info *connection_info){
    struct headerStrip* header = stripHeader(buffer);
    if(header->SEQ != connection_info->remote_seq + connection_info->remote_data_acknowledged + 1){
        return false;
    }

    if(header->ACK != connection_info->my_seq + connection_info->data_sent + 1){
        return false;
    }
    return true;
}

char* tcpHeaderCreator(int flags, int seq, int ack, char* data){
    char ch1[100];
    char ch2[100];
    char ch3[100];
    char ch4[100];

    sprintf(ch1, "%d", flags);
    char flagsString[200] =  "FLAGS\n";
    strcat(flagsString, ch1);

    sprintf(ch2, "%d", seq);
    char seqString[100] =  "\nSEQ\n";
    strcat(seqString, ch2);

    sprintf(ch3, "%d", ack);
    char ackString[100] =  "\nACK\n";
    strcat(ackString, ch3);

    char dataString[100] =  "\nAPPDATA\n";
    if(data != NULL){
        sprintf(ch4, "%s", data);
        strcat(dataString, ch4);
    }
    strcat(flagsString,seqString);

    strcat(flagsString,ackString);

    strcat(flagsString,dataString);

    char* buffer = malloc(sizeof(flagsString));
    strcpy(buffer, flagsString);

    return buffer;
}

// Function to conduct 3-way handshake with server.  Establishes initial
//   SEQ and ACK numbers.  Returns a tcp_info structure to the MP3Client
//   that can later be passed to TCPSend/Receive.
struct tcp_info* TCPConnect(int sockfd,  struct sockaddr_in servaddr){

    struct tcp_info *initTCP = malloc(sizeof(struct tcp_info));
    time_t t;
    srand((unsigned) time(&t));
    int seq = rand() % 10000;
    initTCP->my_seq = seq;
    char* header = tcpHeaderCreator(2,seq,0,0);
    sendto(sockfd, (const char *) header, strlen(header), 0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    printf("Starting three way handshake\n");
    int n, len = sizeof(servaddr);
    char buffer[MAXLINE]; //buffer to store message from server
    if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0) {
        perror("ERROR");
        printf("Errno: %d. ", errno);
        exit(EXIT_FAILURE);
    }
    printf("Server: \n %s \n", buffer);
    sscanf("%d", buffer);
    const char* s = "ab234cid*(s349*(20kd";
    int FLAG, SEQ, ACK;
    if (3 == sscanf(buffer,
                    "%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d",
                    &FLAG,
                    &SEQ,
                    &ACK))
    {
        printf("%d %d %d\n", FLAG, SEQ, ACK);
    }
    initTCP->remote_seq = SEQ;
    header = tcpHeaderCreator(16,seq + 1, SEQ + 1,0);
    sendto(sockfd, (const char *) header, strlen(header), 0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    initTCP->remote_data_acknowledged = 0;
    initTCP->data_received = 0;
    initTCP->data_sent = 0;
    return initTCP;
}



// Replaces all instances of "recvfrom" in your MP3Client.
// UNIQUE PARAMETERS:
//  appdata: pointer to the buffer where the application data in the packet
//           will be stored (just like recvfrom()).  TCP header info is stripped
//           from the packet before it is returned.
//  connection_info: structure that contains info about current connection
int TCPReceive(int sockfd, char *appdata, int appdata_length, struct sockaddr_in addr,
        struct tcp_info *connection_info){
    int n, len = sizeof(addr);
    char buffer[MAXLINE]; //buffer to store message from server
    if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &addr, &len)) < 0) {
        perror("ERROR");
        printf("Errno: %d. ", errno);
        exit(EXIT_FAILURE);
    }
    if(rejectPacket(buffer, connection_info) == false){
        return -1;
    }
    if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &addr, &len)) < 0) {
        perror("ERROR");
        printf("Errno: %d. ", errno);
        exit(EXIT_FAILURE);
    }
    struct headerStrip* header = stripHeader(buffer);

    connection_info->data_received = connection_info->data_received + strlen(header->data)-1;

    if(rejectPacket(buffer, connection_info) == false){
        return -1;
    }
    printf("%s", header->data);
    char* ackHeader = tcpHeaderCreator(16, connection_info->my_seq+connection_info->data_sent + 1, connection_info->remote_seq+connection_info->data_received + 1, NULL);
    sendto(sockfd, (const char *) ackHeader, strlen(ackHeader), 0, (const struct sockaddr *) &addr,
           sizeof(addr));
    connection_info->remote_data_acknowledged = connection_info->remote_data_acknowledged + strlen(header->data)-1;

    appdata = header->data + 11;
    return 0;
}


// Replaces all instances of "sendto" in your MP3Client.
// UNIQUE PARAMETERS:
//  appdata: pointer to the buffer that contains the application data the
//           MP3Client is trying to send.  TCP header information is added to
//           the beginning of this data before it is sent.
//  connection_info: structure that contains info about current connection
int TCPSend(int sockfd, char* appdata, int appdata_length, struct sockaddr_in addr, struct tcp_info *connection_info){
    char* header = tcpHeaderCreator(16,connection_info->my_seq+connection_info->data_sent + 1,connection_info->remote_seq+connection_info->remote_data_acknowledged + 1,appdata);
    sendto(sockfd, (const char *) header, strlen(header), 0, (const struct sockaddr *) &addr, sizeof(addr));
    connection_info->data_sent = connection_info->data_sent + appdata_length;
}

#endif //TCP_MP3SERVER_TCP_FUNCITONS_H